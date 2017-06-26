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

#ifndef VISUALHOMING_MAP_H
#define VISUALHOMING_MAP_H

#include "visualhoming_snapshot.h"
#include "visualhoming_odometry.h"

#include "subsystems/datalink/telemetry.h"

// Configuration
#ifndef VISUALHOMING_MAX_WAYPOINTS
#define VISUALHOMING_MAX_WAYPOINTS 512
#endif

// Map buffers
extern struct snapshot_t vh_waypoints[VISUALHOMING_MAX_WAYPOINTS];
extern int16_t vh_current_waypoint;

// Telemetry callback
void send_visualhoming_map_update(
		struct transport_tx *trans,
		struct link_device *dev);

#endif
