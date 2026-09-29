# Copyright (c) 2026 Deveritec GmbH
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dectnrpi_nds_hw_selftest)

target_sources(app PRIVATE
	src/main.c
	src/leds_buttons.c
	src/ext_flash.c
	src/gpio_loopback.c
)