/*
 * tachometer.h
 *
 *  This module transforms the opto sensor information to data with standard units.
 *
 *	Features:
 *		- velocity in meters per second and rotations per second
 *		- distance in meter since startup
 *
 *  Created on: 02.12.2016
 *
 *      Author: Tobias Hupel
 *      		Eric Elsner
 *
 *      Reviewed on 02.12.2016 by:
 *      		Tobias Hupel
 *      		Eric Elsner
 *      		Adrian Zentgraf
 *
 *      Extension Reviewed on 26.01.2017 by:
 *      		Franz Lübke
 *      		Tobias Hupel
 */

#ifndef TACHOMETER_H_
#define TACHOMETER_H_

#include "global_clock.h"
#include "enums.h"
#include "opto_interruptor.h"

float tachometer_get_velocity_mps(enum wheel_selector wheel);

float tachometer_get_velocity_rotational_ps(enum wheel_selector wheel);

float tachometer_get_distance_meter(enum wheel_selector wheel);

float tachometer_get_arithemtic_mean_distance_meter();

float tachometer_get_extrapolated_distance_meter(enum wheel_selector wheel);

float tachometer_get_extrapolated_arithemtic_mean_distance_meter();

float tachometer_get_arithemtic_mean_velocity_mps();

void tachometer_initialize();

void tachometer_update();

#endif /* TACHOMETER_H_ */
