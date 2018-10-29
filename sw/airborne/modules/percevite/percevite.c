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
 * @file "modules/percevite/percevite.c"
 * @author Tom van Dijk
 * Obstacle detection and avoidance module for PercEvite
 */

#include "modules/percevite/percevite.h"
#include "modules/percevite/percevite_messages.h"

#include "pprzlink/pprz_transport.h"
#include "mcu_periph/udp.h"
#include "subsystems/datalink/downlink.h"
#include "subsystems/datalink/telemetry.h"

#include "firmwares/rotorcraft/navigation.h"
#include "subsystems/navigation/waypoints.h"

#include "subsystems/abi.h"

#include "generated/modules.h"

#include <stdio.h>

// Send velocity estimate through ABI
#ifndef PERCEVITE_ESTIMATE_VELOCITY
#define PERCEVITE_ESTIMATE_VELOCITY FALSE
#endif

// Velocity estimate variance (default SD 50 cm/s = 0.25)
#ifndef PERCEVITE_VELOCITY_R
#define PERCEVITE_VELOCITY_R 0.25
#endif

// Maximum allowable difference with INS velocity [m/s]
#ifndef PERCEVITE_VELOCITY_MAX_ERROR
#define PERCEVITE_VELOCITY_MAX_ERROR 1.0
#endif

// Max allowed time between any SLAMDunk messages [s]
#ifndef PERCEVITE_TIMEOUT
#define PERCEVITE_TIMEOUT 1.0
#endif

// Max allowed time between velocity updates [s]
#ifndef PERCEVITE_VELOCITY_TIMEOUT
#define PERCEVITE_VELOCITY_TIMEOUT 1.0
#endif

#define INVALID_WAYPOINT 255

struct percevite_t percevite = {
    .timeout = 0.0,
    .time_since_velocity = 0.0,
    .wp = INVALID_WAYPOINT, // Should cause an error when not initialized in flight plan!
};

struct percevite_settings_t percevite_settings = {
  .request_flags = REQUEST_FLAG_ALLOW_HIGHER | REQUEST_FLAG_ALLOW_LOWER, // TODO Check if sensible default
};

struct percevite_logging_t percevite_logging = {
    .velocity = { 0.0, 0.0, 0.0 },
    .request = { 0.0, 0.0, 0.0},
    .request_flags = 0x0,
    .reply = { 0.0, 0.0, 0.0},
    .reply_flags = 0x0,
    .target_wp = INVALID_WAYPOINT,
};

static void slamdunk_init(void);
static void slamdunk_event(void);
static void slamdunk_parse_message(void);
static void slamdunk_send_message(union paparazzi_to_slamdunk_msg_t *msg);

static void send_percevite(struct transport_tx *trans, struct link_device *dev);

void percevite_init(void) {
  slamdunk_init();
  register_periodic_telemetry(DefaultPeriodic, PPRZ_MSG_ID_PERCEVITE, send_percevite);
}

void percevite_periodic(void) {
  // Update timeouts
  percevite.time_since_velocity += PERCEVITE_PERIODIC_PERIOD;
  percevite.timeout += PERCEVITE_PERIODIC_PERIOD;
  if(percevite.timeout > PERCEVITE_TIMEOUT) {
    printf("[percevite] WARNING: Lost communication with SLAMDunk!\n");
  }
  if(percevite.time_since_velocity > PERCEVITE_VELOCITY_TIMEOUT) {
    printf("[percevite] WARNING: Velocity timeout exceeded!\n");
  }
  slamdunk_event(); // HACK
}

// Moved to periodic, see https://github.com/paparazzi/paparazzi/issues/2134
//void percevite_event(void) {
//  slamdunk_event();
//}

static void percevite_on_velocity(union slamdunk_to_paparazzi_msg_t *msg) {
  // Velocity estimate
  printf("[percevite] Velocity: %.1f %.1f %.1f m/s\n",
      msg->vx, msg->vy, msg->vz);
  // Sanity check
  struct FloatVect3 vel_body;
  struct FloatRMat *R = stateGetNedToBodyRMat_f();
  struct NedCoor_f *vel_ltp = stateGetSpeedNed_f();
  MAT33_VECT3_MUL(vel_body, *R, *vel_ltp);
  if(fabsf(msg->vx - vel_body.x) < PERCEVITE_VELOCITY_MAX_ERROR &&
      fabsf(msg->vy - vel_body.y) < PERCEVITE_VELOCITY_MAX_ERROR &&
      fabsf(msg->vz - vel_body.z) < PERCEVITE_VELOCITY_MAX_ERROR) {
    // Velocity seems ok, send to INS
#if PERCEVITE_ESTIMATE_VELOCITY
    AbiSendMsgVELOCITY_ESTIMATE(VEL_PERCEVITE_ID, get_sys_time_usec(),
        msg->vx, msg->vy, msg->vz,
        PERCEVITE_VELOCITY_R, PERCEVITE_VELOCITY_R, PERCEVITE_VELOCITY_R);
#endif
  } else {
    // Sanity check failed
    printf("INS velocity: %.1f %.1f %.1f m/s\n", vel_body.x, vel_body.y, vel_body.z);
    printf("Velocity change too large! Ignoring...\n");
  }
  percevite_logging.velocity.x = msg->vx;
  percevite_logging.velocity.y = msg->vy;
  percevite_logging.velocity.z = msg->vz;
  percevite.time_since_velocity = 0.0; // Want to move this to sanity check ok, but this causes timeout during yaw
}

static void percevite_on_vector(union slamdunk_to_paparazzi_msg_t *msg) {
  printf("Received vector (FRD): x = %f, y = %f, z = %f\n", msg->gx, msg->gy, msg->gz);
  struct NedCoor_f *pos_ned = stateGetPositionNed_f();
  struct FloatRMat *R = stateGetNedToBodyRMat_f();
  struct FloatVect3 vector_frd = { msg->gx, msg->gy, msg->gz };
  struct NedCoor_f vector_ned;
  MAT33_VECT3_TRANSP_MUL(vector_ned, *R, vector_frd);
  struct NedCoor_f wp_ned;
  VECT3_SUM(wp_ned, *pos_ned, vector_ned);
  struct EnuCoor_f wp_enu;
  ENU_OF_TO_NED(wp_enu, wp_ned);

  // Horizontal motion
  // Move waypoint, unless:
  //  - Drone needs to halt at current position (gx, gy, gz = 0)
  //  - Percevite wp has been placed at current position (halted = TRUE)
  //  - Percevite wp is still close to current position. This prevents the drone
  //    halting at a previous wp location, for instance if the Flight Plan used
  //    other nav functions in the meantime. Threshold should not be too small,
  //    as wind may cause small deviations between the drone and wp positions.
  // Not moving the waypoint under these conditions should prevent position
  // drift when the drone is stopped in front of an obstacle.
  static bool halted = FALSE; // TRUE after wp has been set to stop at current location
  struct EnuCoor_f wp_enu_old = { WaypointX(percevite.wp), WaypointY(percevite.wp), WaypointAlt(percevite.wp) };
  struct EnuCoor_f wp_diff;
  VECT3_DIFF(wp_diff, wp_enu, wp_enu_old);
  float dist = VECT3_NORM2(wp_diff);
  if(msg->gx == 0.0 && msg->gy == 0.0 && msg->gz == 0.0) {
    if(!halted) { // Percevite wp has not been set yet
      halted = TRUE;
      waypoint_set_enu(percevite.wp, &wp_enu);
    } else {
      if(dist > SQUARE(0.5)) { // Drone is too far away from percevite wp
        waypoint_set_enu(percevite.wp, &wp_enu);
      } // Else: wp is already set, do *not* move it
    }
  } else { // Drone should follow vector
    halted = FALSE;
    waypoint_set_enu(percevite.wp, &wp_enu);
  }
  NavGotoWaypoint(percevite.wp);

  // Vertical motion
  if(fabsf(pos_ned->z - wp_ned.z) < 0.5) { // Close to target altitude
    NavVerticalAltitudeMode(-wp_ned.z, 0.0);
  } else { // Climb/descent required
    float hspeed = stateGetHorizontalSpeedNorm_f();
    float hvec = sqrtf(SQUARE(vector_ned.x) + SQUARE(vector_ned.y));
    float climb = 0.0;
    if(hspeed > 0.1) {
      climb = -hspeed / hvec * vector_ned.z;
    }
    NavVerticalClimbMode(climb);
  }

  percevite_logging.reply = vector_frd;
  percevite_logging.reply_flags = msg->vector_flags;
}

static void percevite_on_message(union slamdunk_to_paparazzi_msg_t *msg) {
  if(msg->flags & SD_MSG_FLAG_VECTOR) percevite_on_vector(msg);
  if(msg->flags & SD_MSG_FLAG_VELOCITY) percevite_on_velocity(msg);
  percevite.timeout = 0.0;
}

static void send_percevite(struct transport_tx *trans, struct link_device *dev) {
  uint8_t ok = PerceviteOk();
  pprz_msg_send_PERCEVITE(trans, dev, AC_ID,
      &ok,
      &percevite_logging.velocity.x, &percevite_logging.velocity.y, &percevite_logging.velocity.z,
      &percevite.timeout, &percevite.time_since_velocity,
      &percevite.wp, &percevite_logging.target_wp,
      &percevite_logging.request.x, &percevite_logging.request.y, &percevite_logging.request.z,
      &percevite_logging.request_flags,
      &percevite_logging.reply.x, &percevite_logging.reply.y, &percevite_logging.reply.z,
      &percevite_logging.reply_flags);
}


/*
 * Navigation functions
 */
/*
 * Outline:
 * Navigation is performed by moving towards the percevite waypoint instead of
 * the real target. The percevite wp is set at a safe distance from the current
 * position in the direction of the target waypoint.
 * The percevite waypoint position is updated with each call to PerceviteGo or
 * PerceviteStay based on the last received safe distance.
 */

/**
 * Sets the heading towards the target_wp as in nav_set_heading_towards_waypoint,
 * but also returns a bool wether the aircraft is currently pointing in the right
 * direction +- a few degrees.
 * @param target_wp
 * @return pointed at target
 */
static bool aim_at_waypoint(uint8_t target_wp) {
  const float threshold = 0.20; // [rad]
  struct FloatVect2 target = {WaypointX(target_wp), WaypointY(target_wp)};
  struct FloatVect2 pos_diff;
  VECT2_DIFF(pos_diff, target, *stateGetPositionEnu_f());
  // don't change heading if closer than 0.5m to target
  if(VECT2_NORM2(pos_diff) < 0.25) {
    return TRUE; // Currently at waypoint, accept all headings
  }
  float target_heading = atan2f(pos_diff.x, pos_diff.y); // Note: ENU, CW from North
  nav_set_heading_rad(target_heading);
  // Compare to current heading
  float heading_error = target_heading - stateGetNedToBodyEulers_f()->psi; // ENU
  while(heading_error < -M_PI) heading_error += 2*M_PI;
  while(heading_error > M_PI) heading_error -= 2*M_PI;
  heading_error = fabsf(heading_error);
  return heading_error < threshold;
}

/**
 * Assign waypoint to PercEvite module. This waypoint will be used for PerceviteGo
 * and PerceviteStay commands. The waypoint will only be placed in regions that
 * are determined to be safe.
 * @param wp
 * @return
 */
bool PerceviteInit(uint8_t wp) {
  percevite.wp = wp;
  return FALSE; // No looping req'd
}

bool PerceviteGo(uint8_t target_wp) {
  if(aim_at_waypoint(target_wp)) {
    // Find target_wp coordinates in body frame
    struct NedCoor_f *pos = stateGetPositionNed_f();
    struct NedCoor_f wp_pos = { WaypointY(target_wp), WaypointX(target_wp), -WaypointAlt(target_wp) }; // Note: waypoint x, y, z are in ENU!
    struct FloatVect3 diff;
    VECT3_DIFF(diff, wp_pos, *pos);
    struct FloatRMat *R = stateGetNedToBodyRMat_f();
    printf("Rg = [%.2f\t%.2f\t%.2f;\n", R->m[0], R->m[1], R->m[2]);
    printf("      %.2f\t%.2f\t%.2f;\n", R->m[3], R->m[4], R->m[5]);
    printf("      %.2f\t%.2f\t%.2f]\n", R->m[6], R->m[7], R->m[8]);
    struct FloatVect3 target_frd;
    MAT33_VECT3_MUL(target_frd, *R, diff);
    // Send request to SLAMDunk
    struct FloatEulers *eul = stateGetNedToBodyEulers_f();
    union paparazzi_to_slamdunk_msg_t msg = {
        .tx = target_frd.x,
        .ty = target_frd.y,
        .tz = target_frd.z,
        .request_flags = percevite_settings.request_flags,
        .phi = eul->phi,
        .theta = eul->theta,
        .psi = eul->psi,
    };
    slamdunk_send_message(&msg);
    printf("Request tx = %f, ty = %f, tz = %f\n", msg.tx, msg.ty, msg.tz);
    percevite_logging.request = target_frd;
    percevite_logging.request_flags = msg.request_flags;
    // Do nothing else! Move percevite_wp when reply is received
  }
  percevite_logging.target_wp = target_wp;
  return sqrtf(get_dist2_to_waypoint(target_wp)) > ARRIVED_AT_WAYPOINT; // Keep looping until arrived at target_wp
}

bool PerceviteStay(uint8_t target_wp) {
  PerceviteGo(target_wp);
  return TRUE; // Keep looping
}

bool PerceviteOk(void) {
  return percevite.timeout < PERCEVITE_TIMEOUT &&
      percevite.time_since_velocity < PERCEVITE_VELOCITY_TIMEOUT;
}


/*
 * Communication with SLAMDunk
 */

struct slamdunk_t {
  struct link_device *device;      ///< The device which is uses for communication
  struct pprz_transport transport; ///< The transport layer (PPRZ)
  uint8_t msg_buf[128];            ///< Message buffer
  bool msg_available;              ///< If we received a message
};
static struct slamdunk_t slamdunk = {
    .device = (&((PERCEVITE_UDP).device)),
    .msg_available = false
};

static void slamdunk_init(void) {
  printf("[percevite] Initialize pprzlink (UDP) to SLAMDunk... ");
  pprz_transport_init(&slamdunk.transport);
  printf("ok\n");
}

static void slamdunk_event(void) {
  pprz_check_and_parse(slamdunk.device, &slamdunk.transport, slamdunk.msg_buf, &slamdunk.msg_available);
  if(slamdunk.msg_available) {
    slamdunk_parse_message();
    slamdunk.msg_available = FALSE;
  }
}

static void slamdunk_parse_message(void) {
  union slamdunk_to_paparazzi_msg_t *msg;
  switch(slamdunk.msg_buf[3]) { // For pprzlink v2.0
    case PPRZ_MSG_ID_PAYLOAD:
      msg = (union slamdunk_to_paparazzi_msg_t *)&slamdunk.msg_buf[5];
      percevite_on_message(msg);
      break;
    default:
      break;
  }
}

static void slamdunk_send_message(union paparazzi_to_slamdunk_msg_t *msg) {
  pprz_msg_send_PAYLOAD(&(slamdunk.transport.trans_tx), slamdunk.device,
      AC_ID, sizeof(*msg), &(msg->bytes));
}

