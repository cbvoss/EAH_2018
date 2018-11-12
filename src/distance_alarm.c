/*
 * distance_alarm.c
 *
 *  Created on: Jan 25, 2017
 *      Author: Tobias Hupel
 *
 *	Reviewed on 26.01.2017 by
 *				Franz Lübke
 *				Tobias Hupel
 */

#include "distance_alarm.h"
#include "tachometer.h"
#include "enums.h"

/**
 * Initializes a new Distance Alarm using a specific wheel selector for distance calculations.
 * The alarm will be active if the specified distance has been reached. (init_distance + alarm_distance_delta_m)
 *
 * @param distance_alarm
 * 		the distance alarm
 * @param alarm_distance_delta_m
 * 		the alarm distance delta in meter
 * @param use_extrapolation
 * 		is the alarm based on extrapolations
 * @param wheel
 * 		the wheel defining the distance source for the alarm calculation
 */
void distance_alarm_new_specific(struct DistanceAlarm * distance_alarm, float alarm_distance_delta_m, char use_extrapolation,
		enum wheel_selector wheel)
{
	distance_alarm->init_distance_m =
			use_extrapolation ? tachometer_get_extrapolated_distance_meter(wheel) : tachometer_get_distance_meter(wheel);
	distance_alarm->alarm_distance_delta_m = alarm_distance_delta_m;
	distance_alarm->wheel = wheel;
	distance_alarm->use_extrapolation = use_extrapolation;
	distance_alarm->specific_wheel = 1;
}

/**
 * Initializes a new Distance Alarm using the arithmetic mean distance of both wheels for distance calculations.
 * The alarm will be active if the specified distance has been reached. (init_distance + alarm_distance_delta_m)
 *
 * @param distance_alarm
 * 		the distance alarm
 * @param alarm_distance_delta_m
 * 		the alarm distance delta in meter
 * @param use_extrapolation
 * 		is the alarm based on extrapolations
 */
void distance_alarm_new_mean(struct DistanceAlarm * distance_alarm, float alarm_distance_delta_m, char use_extrapolation)
{
	distance_alarm->init_distance_m =
			use_extrapolation ?
					tachometer_get_extrapolated_arithemtic_mean_distance_meter() : tachometer_get_arithemtic_mean_distance_meter();
	distance_alarm->alarm_distance_delta_m = alarm_distance_delta_m;
	distance_alarm->wheel = FRONT_RIGHT;
	distance_alarm->use_extrapolation = use_extrapolation;
	distance_alarm->specific_wheel = 0;
}

/**
 * Resets the init_distance of the given Distance Alarm to the current distance depending of the Distance Alarm's configuration.
 *
 * @param distance_alarm
 * 		the distance alarm
 */
void distance_alarm_reset(struct DistanceAlarm * distance_alarm)
{
	if (distance_alarm->use_extrapolation)
	{
		if (distance_alarm->specific_wheel)
		{
			distance_alarm->init_distance_m = tachometer_get_extrapolated_distance_meter(distance_alarm->wheel);
		}
		else
		{
			distance_alarm->init_distance_m = tachometer_get_extrapolated_arithemtic_mean_distance_meter();
		}
	}
	else
	{
		if (distance_alarm->specific_wheel)
		{
			distance_alarm->init_distance_m = tachometer_get_distance_meter(distance_alarm->wheel);
		}
		else
		{
			distance_alarm->init_distance_m = tachometer_get_arithemtic_mean_distance_meter();
		}
	}
}

/**
 * Returns the active state of the given Distance Alarm. Either "1" if the distance has been reached or "0" if not.
 *
 * @param distance_alarm
 * 		the distance alarm
 * @return
 * 		the active state
 */
char distance_alarm_has_distance_reached(struct DistanceAlarm * distance_alarm)
{
	float currentDistance;

	if (distance_alarm->use_extrapolation)
	{
		if (distance_alarm->specific_wheel)
		{
			currentDistance = tachometer_get_extrapolated_distance_meter(distance_alarm->wheel);
		}
		else
		{
			currentDistance = tachometer_get_extrapolated_arithemtic_mean_distance_meter();
		}
	}
	else
	{
		if (distance_alarm->specific_wheel)
		{
			currentDistance = tachometer_get_distance_meter(distance_alarm->wheel);
		}
		else
		{
			currentDistance = tachometer_get_arithemtic_mean_distance_meter();
		}
	}

	return currentDistance >= (distance_alarm->init_distance_m + distance_alarm->alarm_distance_delta_m);
}

