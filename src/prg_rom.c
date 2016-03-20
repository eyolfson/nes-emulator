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

#include "prg_rom.h"

static uint8_t *bank_1_data;
static uint8_t *bank_2_data;

void prg_rom_set_bank_1(uint8_t *data)
{
	bank_1_data = data;
}

void prg_rom_set_bank_2(uint8_t *data)
{
	bank_2_data = data;
}

uint8_t prg_rom_read_bank_1(uint16_t offset)
{
	return bank_1_data[offset];
}

uint8_t prg_rom_read_bank_2(uint16_t offset)
{
	return bank_2_data[offset];
}
