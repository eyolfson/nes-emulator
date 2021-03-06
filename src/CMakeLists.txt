# Copyright 2016 Jonathan Eyolfson
#
# This program is free software: you can redistribute it and/or modify it under
# the terms of the GNU General Public License version 3 as published by the Free
# Software Foundation.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program. If not, see <http://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.1.3)

project(NES_EMULATOR C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wpedantic)
add_compile_options(-Wall)
add_compile_options(-Wextra)

find_package(PkgConfig)

pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(WAYLAND_CLIENT REQUIRED wayland-client)
pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
pkg_get_variable(WAYLAND_PROTOCOLS_DATADIR wayland-protocols pkgdatadir)
pkg_check_modules(WAYLAND_SCANNER REQUIRED wayland-scanner)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
	COMMAND wayland-scanner
	ARGS client-header
	     ${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-shell/xdg-shell-unstable-v6.xml
	     ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
)

add_custom_command(
	OUTPUT ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.c
	COMMAND wayland-scanner
	ARGS code
	     ${WAYLAND_PROTOCOLS_DATADIR}/unstable/xdg-shell/xdg-shell-unstable-v6.xml
	     ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.c
)

include_directories(
	${CMAKE_BINARY_DIR}
	${ALSA_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	${LIBEVDEV_INCLUDE_DIRS}
	${WAYLAND_CLIENT_INCLUDE_DIRS}
)

add_executable(nes-emulator
	main.c

	apu.c
	args.c
	cartridge.c
	console.c
	controller.c
	cpu.c
	exit_code.c
	ppu.c
	ppu_register.c

	backend/wayland.c
	backend/wayland_buffer.c
	backend/wayland_ppu.c

	backend/alsa.c

	backend/evdev.c

	${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.c
	${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
)

target_link_libraries(nes-emulator
	${ALSA_LIBRARIES}
	${CAIRO_LIBRARIES}
	${LIBEVDEV_LIBRARIES}
	${WAYLAND_CLIENT_LIBRARIES}
)
