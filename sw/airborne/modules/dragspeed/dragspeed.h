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
/**
 * @file "modules/dragspeed/dragspeed.h"
 * @author Tom van Dijk
 * This module estimates the velocity of rotorcraft by measuring the drag force using the accelerometer.
 */

#ifndef DRAGSPEED_H
#define DRAGSPEED_H

#include "math/pprz_algebra_float.h"

struct dragspeed_t {
	// Estimated velocity
	struct FloatVect2 vel;
	// Low-pass filter
	float filter;
	// Drag coefficient calibration
	float coeff;
	int do_calibrate_coeff;
};
extern struct dragspeed_t dragspeed;

extern void dragspeed_init(void);

#endif

