/*
 * distance_alarm.h
 *
 *  Created on: Jan 25, 2017
 *      Author: Tobias Hupel
 *
 *	Reviewed on 26.01.2017 by
 *				Franz Lübke
 *				Tobias Hupel
 */

#ifndef DISTANCE_ALARM_H_
#define DISTANCE_ALARM_H_

#include "enums.h"

struct DistanceAlarm
{
	char use_extrapolation, specific_wheel;
	float init_distance_m, alarm_distance_delta_m;
	enum wheel_selector wheel;
};

void distance_alarm_new_specific(struct DistanceAlarm * distance_alarm, float alarm_distance_delta_m, char use_extrapolation,
		enum wheel_selector wheel);

void distance_alarm_new_mean(struct DistanceAlarm * distance_alarm, float alarm_distance_delta_m, char use_extrapolation);

void distance_alarm_reset(struct DistanceAlarm * distance_alarm);

char distance_alarm_has_distance_reached(struct DistanceAlarm * distance_alarm);

#endif /* DISTANCE_ALARM_H_ */
