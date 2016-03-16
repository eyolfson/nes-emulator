/*
 * Copyright 2016 Jonathan Eyolfson
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "cpu.h"

#include "exit_code.h"

#include <stdbool.h>

uint8_t memory[MEMORY_SIZE];

void init_registers(struct registers *registers)
{
	registers->a = 0;
	registers->x = 0;
	registers->y = 0;
	registers->p = 0x24;
	registers->s = 0xFD;
	registers->pc = 0xC000;
}

static
void
set_carry_flag(struct registers *registers, bool c)
{
	if (c) {
		registers->p |= 1 << 0;
	}
	else {
		registers->p &= ~(1 << 0);
	}
}

static
bool
get_carry_flag(struct registers *registers)
{
	return registers->p & (1 << 0);
}

static
void
set_zero_flag(struct registers *registers, bool z)
{
	if (z) {
		registers->p |= 1 << 1;
	}
	else {
		registers->p &= ~(1 << 1);
	}
}

static
bool
get_zero_flag(struct registers *registers)
{
	return registers->p & 1 << 1;
}

static
void
set_interrupt_disable_flag(struct registers *registers, bool i)
{
	if (i) {
		registers->p |= 1 << 2;
	}
	else {
		registers->p &= ~(1 << 2);
	}
}

static
bool
get_interrupt_disable_flag(struct registers *registers)
{
	return registers->p & 1 << 2;
}

static
void
set_decimal_mode_flag(struct registers *registers, bool d)
{
	if (d) {
		registers->p |= 1 << 3;
	}
	else {
		registers->p &= ~(1 << 3);
	}
}

static
bool
get_decimal_mode_flag(struct registers *registers)
{
	return registers->p & 1 << 3;
}

static
void
set_break_command_flag(struct registers *registers, bool b)
{
	if (b) {
		registers->p |= 1 << 4;
	}
	else {
		registers->p &= ~(1 << 4);
	}
}

static
bool
get_break_command_flag(struct registers *registers)
{
	return registers->p & 1 << 4;
}

static
void
set_overflow_flag(struct registers *registers, bool v)
{
	if (v) {
		registers->p |= 1 << 6;
	}
	else {
		registers->p &= ~(1 << 6);
	}
}

static
bool
get_overflow_flag(struct registers *registers)
{
	return registers->p & 1 << 6;
}

static
void
set_negative_flag(struct registers *registers, bool n)
{
	if (n) {
		registers->p |= 1 << 7;
	}
	else {
		registers->p &= ~(1 << 7);
	}
}

static
bool
get_negative_flag(struct registers *registers)
{
	return registers->p & 1 << 7;
}

static
void
push_to_stack(struct registers *registers, uint8_t v)
{
	*(memory + 0x0100 + registers->s) = v;
	if (registers->s == 0x00) {
		registers->s = 0xFF;
	}
	else {
		registers->s -= 1;
	}
}

static
uint8_t
pop_from_stack(struct registers *registers)
{
	if (registers->s == 0xFF) {
		registers->s = 0x00;
	}
	else {
		registers->s += 1;
	}
	return *(memory + 0x0100 + registers->s);
}

static
void
write_to_zero_page(uint8_t offset, uint8_t v)
{
	*(memory + offset) = v;
}

static
void
write_to_memory(uint16_t offset, uint8_t v)
{
	*(memory + offset) = v;
}

static
uint8_t
read_from_zero_page(uint8_t offset)
{
	return *(memory + offset);
}

static
uint8_t
read_from_memory(uint16_t offset)
{
	return *(memory + offset);
}

static
void
execute_compare(struct registers *registers, uint8_t v1, uint8_t v2)
{
	uint16_t t = v1 - v2;
	set_carry_flag(registers, t < 0x100);
	set_zero_flag(registers, t == 0);
	set_negative_flag(registers, t & (1 << 7));
}

static
void
execute_add_with_carry(struct registers *registers, uint8_t v)
{
	int8_t a = (int8_t) registers->a;
	int8_t b = (int8_t) v;

	int16_t result = a + b;
	bool inc_carry = false;
	if (get_carry_flag(registers)) {
		if (result == -1) {
			inc_carry = true;
		}
		result += 1;
	}

	if (inc_carry) {
		set_carry_flag(registers, true);
	}
	else if (result > 0) {
		set_carry_flag(registers, result & 0x100);
	}
	else {
		set_carry_flag(registers, false);
	}

	/* If the operands have opposite signs, the sum will never overflow */
	if (a >= 0 && b >= 0 && (int8_t) result < 0) {
		set_overflow_flag(registers, true);
	}
	else if (a < 0 && b < 0 && (int8_t) result > 0) {
		set_overflow_flag(registers, true);
	}
	else {
		set_overflow_flag(registers, false);
	}

	set_negative_flag(registers, result & 0x80);
	set_zero_flag(registers, result == 0);
	registers->a = (result & 0xFF);
}

static
void
execute_subtract_with_carry(struct registers *registers, uint8_t v)
{
	int8_t a = (int8_t) registers->a;
	int8_t b = (int8_t) v;

	int16_t result = a - b;
	uint16_t t = registers->a + ((uint8_t) (~v) + (uint8_t) 1);
	if (!get_carry_flag(registers)) {
		result -= 1;
		t += 0xFF;
	}

	set_carry_flag(registers, t >= 0x100);
	set_overflow_flag(registers, result < -128 || result > 127);
	set_negative_flag(registers, result & 0x80);
	set_zero_flag(registers, result == 0);
	registers->a = (result & 0xFF);
}

uint8_t execute_instruction(struct registers *registers)
{
	uint8_t *memory_pc_offset = memory + registers->pc;
	uint8_t opcode = *(memory_pc_offset);

	uint16_t t1 = 0;
	uint16_t t2 = 0;

	/* Processor is little-endian */
	switch (opcode) {
	case 0x01:
		/* ORA - Logical Inclusive OR */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);
		t2 = registers->a | t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x05:
		/* ORA - Logical Inclusive OR */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);

		t1 = registers->a | t2;

		registers->a = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x08:
		/* PHP - Push Processor Status */
		/* Bytes: 1 */
		/* Cycles: 3 */
		push_to_stack(registers, registers->p);
		registers->pc += 1;
		break;
	case 0x09:
		/* ORA - Logical Inclusive OR */
		/* Addressing is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		t2 = registers->a | t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x0A:
		/* ASL - Arithmetic Shift Left */
		/* Addressing is accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_carry_flag(registers, registers->a & 0x80);
		registers->a = registers->a << 1;
		set_negative_flag(registers, registers->a & 0x80);
		set_zero_flag(registers, registers->a == 0);
		registers->pc += 1;
		break;
	case 0x10:
		/* BPL - Branch if Positive */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (!get_negative_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0x18:
		/* CLC - Clear Carry Flag */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_carry_flag(registers, false);
		registers->pc += 1;
		break;
	case 0x20:
		/* JSR - Jump to Subroutine */
		/* Address is absolute */
		/* Bytes: 3 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);

		t2 = registers->pc + 3 - 1;
		push_to_stack(registers, (t2 & 0xFF00) >> 8);
		push_to_stack(registers, t2 & 0x00FF);

		registers->pc = t1;
		break;
	case 0x21:
		/* AND - Logical AND */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);
		t2 = registers->a & t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x24:
		/* BIT - Bit Test */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);

		set_zero_flag(registers, (registers->a & t2) == 0);
		set_overflow_flag(registers, t2 & (1 << 6));
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x25:
		/* AND - Logical AND */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);

		t1 = registers->a & t2;

		registers->a = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x28:
		/* PLP - Pull Processor Status */
		/* Bytes: 1 */
		/* Cycles: 4 */

		/* TODO: perserve break command flag? */
		t1 = get_break_command_flag(registers);
		registers->p = pop_from_stack(registers);
		/* TODO: flag seems to always be set? */
		registers->p |= 0x20;
		if (t1) {
			set_break_command_flag(registers, true);
		}
		else {
			set_break_command_flag(registers, false);
		}
		registers->pc += 1;
		break;
	case 0x29:
		/* AND - Logical AND */
		/* Addressing is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		t2 = registers->a & t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x2A:
		/* ROL - Rotate Left */
		/* Addressing is accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		t1 = get_carry_flag(registers);
		set_carry_flag(registers, registers->a & 0x80);
		registers->a = registers->a << 1;
		if (t1) {
			registers->a |= 0x01;
		}
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 1;
		break;
	case 0x30:
		/* BMI - Branch if Minus */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (get_negative_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0x38:
		/* SEC - Set Carry Flag */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_carry_flag(registers, true);
		registers->pc += 1;
		break;
	case 0x40:
		/* RTI - Return from Interrupt */
		/* Bytes: 1 */
		/* Cycles: 6 */
		t1 = get_break_command_flag(registers);
		registers->p = pop_from_stack(registers);
		/* TODO: flag seems to always be set? */
		registers->p |= 0x20;
		if (t1) {
			set_break_command_flag(registers, true);
		}
		else {
			set_break_command_flag(registers, false);
		}
		t2 = pop_from_stack(registers);
		t2 += pop_from_stack(registers) << 8;
		registers->pc = t2;
		break;
	case 0x41:
		/* EOR - Exclusive OR */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);
		t2 = registers->a ^ t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x45:
		/* EOR - Exclusive OR */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);

		t1 = registers->a ^ t2;

		registers->a = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x48:
		/* PHA - Push Accumulator */
		/* Bytes: 1 */
		/* Cycles: 3 */
		push_to_stack(registers, registers->a);
		registers->pc += 1;
		break;
	case 0x49:
		/* EOR - Exclusive OR */
		/* Addressing is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		t2 = registers->a ^ t1;

		registers->a = t2;
		set_zero_flag(registers, t2 == 0);
		set_negative_flag(registers, t2 & (1 << 7));

		registers->pc += 2;
		break;
	case 0x4A:
		/* LSR - Logical Shift Right */
		/* Addressing is accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_carry_flag(registers, registers->a & 0x01);
		set_negative_flag(registers, false);
		registers->a = registers->a >> 1;
		set_zero_flag(registers, registers->a == 0);
		registers->pc += 1;
		break;
	case 0x4C:
		/* JMP - Jump */
		/* Address is absolute */
		/* Bytes: 3 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		registers->pc = t1;
		break;
	case 0x50:
		/* BVC - Branch if Overflow Clear */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (!get_overflow_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0x60:
		/* RTS - Return from Subroutine */
		/* Bytes: 1 */
		/* Cycles: 6 */
		t1 = pop_from_stack(registers);
		t1 += pop_from_stack(registers) << 8;
		t1 += 1;
		registers->pc = t1;
		break;
	case 0x61:
		/* ADC - Add with Carry */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);

		execute_add_with_carry(registers, t1);
		registers->pc += 2;
		break;
	case 0x65:
		/* ADC - Add with Carry */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);
		execute_add_with_carry(registers, t2);
		registers->pc += 2;
		break;
	case 0x68:
		/* PLA - Pull Accumulator */
		/* Bytes: 1 */
		/* Cycles: 4 */
		registers->a = pop_from_stack(registers);
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 1;
		break;
	case 0x69:
		/* ADC - Add with Carry */
		/* Addressing is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		execute_add_with_carry(registers, t1);
		registers->pc += 2;
		break;
	case 0x6A:
		/* ROR - Rotate Right */
		/* Addressing is accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		t1 = get_carry_flag(registers);
		set_carry_flag(registers, registers->a & 0x01);
		registers->a = registers->a >> 1;
		if (t1) {
			registers->a |= 0x80;
		}
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 1;
		break;
	case 0x6C:
		/* JMP - Jump */
		/* Address is indirect */
		/* Bytes: 3 */
		/* Cycles: 5 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		t2 = *(memory + t1) + (*(memory + t1 + 1) << 8);
		registers->pc = t2;
		break;
	case 0x70:
		/* BVS - Branch if Overflow Set */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (get_overflow_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0x78:
		/* SEI - Set Interrupt Disable */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_interrupt_disable_flag(registers, true);
		registers->pc += 1;
		break;
	case 0x81:
		/* STA - Store Accumulator */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		write_to_memory(t2, registers->a);
		registers->pc += 2;
		break;
	case 0x84:
		/* STY - Store Y Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		write_to_zero_page(t1, registers->y);
		registers->pc += 2;
		break;
	case 0x85:
		/* STA - Store Accumulator */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		write_to_zero_page(t1, registers->a);
		registers->pc += 2;
		break;
	case 0x86:
		/* STX - Store X Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		write_to_zero_page(t1, registers->x);
		registers->pc += 2;
		break;
	case 0x88:
		/* DEY - Decrement Y Register */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->y -= 1;
		set_zero_flag(registers, registers->y == 0);
		set_negative_flag(registers, registers->y & (1 << 7));
		registers->pc += 1;
		break;
	case 0x8A:
		/* TXA - Transfer X to Accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->a = registers->x;
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 1;
		break;
	case 0x8D:
		/* STA - Store Accumulator */
		/* Addressing is absolute */
		/* Bytes: 3 */
		/* Cycles: 4 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		write_to_memory(t1, registers->a);
		registers->pc += 3;
		break;
	case 0x8E:
		/* STX - Store X Register */
		/* Addressing is absolute */
		/* Bytes: 3 */
		/* Cycles: 4 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		write_to_memory(t1, registers->x);
		registers->pc += 3;
		break;
	case 0x90:
		/* BCC - Branch if Carry Clear */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (!get_carry_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0x98:
		/* TYA - Transfer Y to Accumulator */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->a = registers->y;
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 1;
		break;
	case 0x9A:
		/* TXS - Transfer X to Stack Pointer */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->s = registers->x;
		registers->pc += 1;
		break;
	case 0xA0:
		/* LDX - Load X Register */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		registers->y = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA1:
		/* LDA - Load Accumlator */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		registers->a = read_from_memory(t2);
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA2:
		/* LDX - Load X Register */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		registers->x = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA4:
		/* LDY - Load Y Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		registers->y = read_from_zero_page(t1);
		set_zero_flag(registers, registers->y == 0);
		set_negative_flag(registers, registers->y & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA5:
		/* LDA - Load Accumlator */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		registers->a = read_from_zero_page(t1);
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA6:
		/* LDX - Load X Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		registers->x = read_from_zero_page(t1);
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 2;
		break;
	case 0xA8:
		/* TAY - Transfer Accumulator to Y */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->y = registers->a;
		set_zero_flag(registers, registers->y == 0);
		set_negative_flag(registers, registers->y & (1 << 7));
		registers->pc += 1;
		break;
	case 0xA9:
		/* LDA - Load Accumlator */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		registers->a = t1;
		set_zero_flag(registers, t1 == 0);
		set_negative_flag(registers, t1 & (1 << 7));
		registers->pc += 2;
		break;
	case 0xAA:
		/* TAY - Transfer Accumulator to X */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->x = registers->a;
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 1;
		break;
	case 0xAD:
		/* LDA - Load Acuumulator */
		/* Addressing is absolute */
		/* Bytes: 3 */
		/* Cycles: 4 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		registers->a = read_from_memory(t1);
		set_zero_flag(registers, registers->a == 0);
		set_negative_flag(registers, registers->a & (1 << 7));
		registers->pc += 3;
		break;
	case 0xAE:
		/* LDX - Load X Register */
		/* Addressing is absolute */
		/* Bytes: 3 */
		/* Cycles: 4 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		registers->x = read_from_memory(t1);
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 3;
		break;
	case 0xB0:
		/* BCS - Branch if Carry Set */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (get_carry_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0xB8:
		/* CLV - Clear Overflow Flag */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_overflow_flag(registers, false);
		registers->pc += 1;
		break;
	case 0xBA:
		/* TSX - Transfer Stack Pointer to X */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->x = registers->s;
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 1;
		break;
	case 0xC0:
		/* CPY - Compare Y Register */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		execute_compare(registers, registers->y, t1);
		registers->pc += 2;
		break;
	case 0xC1:
		/* CMP - Compare */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);

		execute_compare(registers, registers->a, t1);
		registers->pc += 2;
		break;
	case 0xC4:
		/* CPY - Compare Y Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);
		execute_compare(registers, registers->y, t2);
		registers->pc += 2;
		break;
	case 0xC5:
		/* CMP - Compare */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);
		execute_compare(registers, registers->a, t2);
		registers->pc += 2;
		break;
	case 0xC8:
		/* INY - Increment Y Register */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->y += 1;
		set_zero_flag(registers, registers->y == 0);
		set_negative_flag(registers, registers->y & (1 << 7));
		registers->pc += 1;
		break;
	case 0xC9:
		/* CMP - Compare */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		execute_compare(registers, registers->a, t1);
		registers->pc += 2;
		break;
	case 0xCA:
		/* DEX - Decrement X Register */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->x -= 1;
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 1;
		break;
	case 0xCC:
		/* CPY - Compare Y Register */
		/* Address is absolute */
		/* Bytes: 3 */
		/* Cycles: 4 */
		t1 = *(memory_pc_offset + 1) + (*(memory_pc_offset + 2) << 8);
		t2 = read_from_memory(t1);
		execute_compare(registers, registers->y, t2);
		registers->pc += 3;
		break;
	case 0xD0:
		/* BNE - Branch if Not Equal */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (!get_zero_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0xD8:
		/* CLD - Clear Decimal Mode */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_decimal_mode_flag(registers, false);
		registers->pc += 1;
		break;
	case 0xE0:
		/* CPX - Compare X Register */
		/* Operand is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		execute_compare(registers, registers->x, t1);
		registers->pc += 2;
		break;
	case 0xE1:
		/* SBC - Subtract with Carry */
		/* Addressing is (Indirect, X) */
		/* Bytes: 2 */
		/* Cycles: 6 */
		t1 = *(memory_pc_offset + 1);
		t1 += registers->x;
		t1 &= 0x00FF;

		t2 = read_from_zero_page(t1);
		t2 += read_from_zero_page(t1 + 1) << 8;

		t1 = read_from_memory(t2);

		execute_subtract_with_carry(registers, t1);
		registers->pc += 2;
		break;
	case 0xE4:
		/* CPX - Compare X Register */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);
		execute_compare(registers, registers->x, t2);
		registers->pc += 2;
		break;
	case 0xE5:
		/* SBC - Subtract with Carry */
		/* Addressing is zero page */
		/* Bytes: 2 */
		/* Cycles: 3 */
		t1 = *(memory_pc_offset + 1);
		t2 = read_from_zero_page(t1);
		execute_subtract_with_carry(registers, t2);
		registers->pc += 2;
		break;
	case 0xE8:
		/* INX - Increment X Register */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->x += 1;
		set_zero_flag(registers, registers->x == 0);
		set_negative_flag(registers, registers->x & (1 << 7));
		registers->pc += 1;
		break;
	case 0xE9:
		/* SBC - Subtract with Carry */
		/* Addressing is immediate */
		/* Bytes: 2 */
		/* Cycles: 2 */
		t1 = *(memory_pc_offset + 1);
		execute_subtract_with_carry(registers, t1);
		registers->pc += 2;
		break;
	case 0xEA:
		/* NOP - No Operation */
		/* Bytes: 1 */
		/* Cycles: 2 */
		registers->pc += 1;
		break;
	case 0xF0:
		/* BEQ - Branch if Equal */
		/* Addressing is relative */
		/* Bytes: 2 */
		/* Cycles: 2 (+1 if branch succeeds, +2 if to a new page) */
		registers->pc += 2;
		if (get_zero_flag(registers)) {
			t1 = *(memory_pc_offset + 1);
			registers->pc += t1;
		}
		break;
	case 0xF8:
		/* SED - Set Decimal Flag */
		/* Bytes: 1 */
		/* Cycles: 2 */
		set_decimal_mode_flag(registers, true);
		registers->pc += 1;
		break;
	default:
		return EXIT_CODE_UNIMPLEMENTED_BIT;
	}

	return 0;
}