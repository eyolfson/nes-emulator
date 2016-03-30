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

#include "ppu.h"

static uint8_t ctrl = 0;
static uint8_t status = 0;

uint8_t ppu_read(uint8_t address)
{
printf("read %x\n", address);
	switch (address) {
	case 0:
		return ctrl;
	case 2:
		return 0xE0;
	}
	return 0;
}

void ppu_write(uint8_t address, uint8_t value)
{
printf("write %x %02X\n", address, value);
	switch (address) {
	case 0:
		ctrl = value;
		break;
	case 2:
		break;
	}
}
