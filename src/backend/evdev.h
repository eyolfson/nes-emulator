/*
 * Copyright 2017 Jonathan Eyolfson
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License version 3 as published by the
 * Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../controller.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint8_t nes_emulator_backend_evdev_init(
	struct nes_emulator_controller_backend **controller_backend);
uint8_t nes_emulator_backend_evdev_fini(
	struct nes_emulator_controller_backend **controller_backend);

#ifdef __cplusplus
}
#endif

