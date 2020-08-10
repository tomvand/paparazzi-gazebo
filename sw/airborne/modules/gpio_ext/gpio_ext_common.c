/*
 * Copyright (C) Tom van Dijk <tomvand@users.noreply.github.com>
 *
 * This file is part of paparazzi
 *
 * paparazzi is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * paparazzi is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with paparazzi; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/** @file "modules/gpio_ext/gpio_ext_common.c"
 * @author Tom van Dijk <tomvand@users.noreply.github.com>
 * Common external GPIO functions.
 */

#include "modules/gpio_ext/gpio_ext_common.h"

#include "mcu_periph/gpio.h"
#include "led.h"

#include <stdint.h>

void gpio_ext_common_init(void)
{
  // your init code here
}

void gpio_ext_common_event(void)
{
  // your event code here
}

// Wrapping functions
void __wrap_gpio_set(uint32_t port, uint32_t gpios);
void __real_gpio_set(uint32_t port, uint32_t gpios);
void __wrap_gpio_set(uint32_t port, uint32_t gpios) {
  if (port >= GPIOEXT1 && port <= GPIOEXT4) {
    // Do magic
  } else {
    __real_gpio_set(port, gpios);
  }
}

