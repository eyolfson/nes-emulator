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

#include "console.h"

#include <stdlib.h>

#include "cpu.h"
#include "exit_code.h"

struct nes_emulator_console {
	struct registers registers;
};

uint8_t nes_emulator_console_init(struct nes_emulator_console **console)
{
	*console = malloc(sizeof(struct nes_emulator_console));
	return 0;
}

uint8_t nes_emulator_console_fini(struct nes_emulator_console **console)
{
	free(*console);
	*console = NULL;
	return 0;
}