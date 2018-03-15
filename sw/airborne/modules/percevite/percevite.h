/*
 * Copyright (C) Tom van Dijk
 * Based on slamdunk_udp by kevindehecker
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
 * @file "modules/percevite/percevite.h"
 * @author Tom van Dijk
 * Obstacle detection and avoidance module for PercEvite
 */

#ifndef PERCEVITE_H
#define PERCEVITE_H

#include "firmwares/rotorcraft/navigation.h" // No idea where the bool type comes from :S
#include <stdint.h>

struct percevite_t {
  float safe_distance; ///< Distance that can be moved forward whilst maintaining the minimal safe distance.
  float max_velocity;
  float time_since_image;
};
extern struct percevite_t percevite;

struct percevite_settings_t {
  float deceleration;
  float minimum_distance;
  float image_lag;
  float pixels_threshold;
};
extern struct percevite_settings_t percevite_settings;

extern void percevite_init(void);
extern void percevite_periodic(void);
extern void percevite_event(void);

extern bool PerceviteInit(uint8_t wp);
extern bool PerceviteGo(uint8_t target_wp);
extern bool PerceviteStay(uint8_t target_wp);

#endif

