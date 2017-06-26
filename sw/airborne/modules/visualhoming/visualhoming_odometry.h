/*
 * Copyright (C) Tom van Dijk
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

#ifndef VISUALHOMING_ODOMETRY_H
#define VISUALHOMING_ODOMETRY_H

#include "visualhoming_snapshot.h"

extern float vh_odo_update_threshold;

struct odometry_t {
	float x;
	float y;
};

extern struct odometry_t tel_ss_ref_odo;

void vh_odometry_reset(
		struct odometry_t *odo,
		const struct snapshot_t *current_ss);
void vh_odometry_update(
		struct odometry_t *odo,
		const struct snapshot_t *current_ss);

#endif