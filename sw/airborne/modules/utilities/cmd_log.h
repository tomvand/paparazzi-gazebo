/*
 * Copyright (C) Tom van Dijk <tomvand@noreply.users.github.com>
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

/** @file "modules/utilities/cmd_log.h"
 * @author Tom van Dijk <tomvand@noreply.users.github.com>
 * Log intermediate values in the command_laws section.
 */

#ifndef CMD_LOG_H
#define CMD_LOG_H

#include "paparazzi.h"
#include "generated/airframe.h"


extern pprz_t cmd_log[COMMANDS_NB];

extern void cmd_log_values(bool motors_on, bool override_on, pprz_t in_cmd[]);

#endif  // CMD_LOG_H
