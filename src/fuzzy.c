/*
 * fuzzy.c
 *
 *  Created on: Dec 7, 2017
 *      Author: Tobias Hupel
 */

#include "fuzzy.h"
#include "ir_pattern_tools.h"
#include "distance_alarm.h"
#include "Ir_Board.h"
#include "line_buffer.h"
#include "global_clock.h"
#include "servo.h"
#include "serial_blue.h"
#include "math_tools.h"
#include "tachometer.h"
#include <stdio.h>
#include <math.h>

#include "line_abs_9SI_V_F4.h"
#include "line_abs_9SI_F4.h"
#include "target_speed_F4.h"

#define LINE_INPUT_MAX_NATURAL_VALUE 3.5f
#define LINE_INPUT_MAX_PROCESSABLE_VALUE 4.0f
#define MAX_ONES_COUNT 3
#define FUZZY_INPUT_TRUST_THRESHOLD 3.0f

//#define TRACE_FUZZY
//#define TRACE_CALCULATED_ANGLE

struct LineBuffer g_fuzzyLineBuffer;
struct DistanceAlarm g_fuzzyDistanceAlarm;

#if defined(TRACE_FUZZY)
struct timer g_fuzzyDebugTimer;
#endif

FuzzyDeviationReturnStatus calculate_fuzzy_line_deviation(unsigned char irPattern, float currentServoAngle_deg, float* deviation);

/**
 * Initializes WinFACT Fuzzy regulators.
 */
void fuzzy_initialize()
{
	line_buffer_new(&g_fuzzyLineBuffer);
	distance_alarm_new_mean(&g_fuzzyDistanceAlarm, 0.001f, 1);

	target_speed_F4_init();
	line_abs_9SI_V_F4_init();
	line_abs_9SI_F4_init();

#if defined(TRACE_FUZZY)
	global_clock_set_timer(&g_fuzzyDebugTimer, 20);
#endif
}

void fuzzy_update()
{
	if (distance_alarm_has_distance_reached(&g_fuzzyDistanceAlarm))
	{
		distance_alarm_reset(&g_fuzzyDistanceAlarm);

		line_buffer_add_pattern(&g_fuzzyLineBuffer, ir_get_pattern());
	}

#if defined(TRACE_FUZZY)
	if (global_clock_timer(&g_fuzzyDebugTimer))
	{
		global_clock_reset_timer(&g_fuzzyDebugTimer);

		char buff[64];

		float deviation = 0;
		calculate_fuzzy_line_deviation(ir_get_pattern(), servo_get_position_angle_deg(), &deviation);
		float resultAngle;

#ifdef TRACE_CALCULATED_ANGLE
		resultAngle = fuzzy_calculate_servo_angle(ir_get_pattern(), servo_get_position_angle_deg(), tachometer_get_arithemtic_mean_velocity_mps());
#else
		resultAngle = servo_get_position_angle_deg();
#endif
		sprintf(buff, "%f;%f;%f;%f;%f\n", deviation, line_buffer_calculate_average(&g_fuzzyLineBuffer, 100) - LINE_INPUT_MAX_NATURAL_VALUE,
				line_buffer_calculate_gradient(&g_fuzzyLineBuffer, 100) * 100, resultAngle, tachometer_get_arithemtic_mean_velocity_mps());

		serial_blue_write_string(buff);
	}
#endif
}

/**
 * Calculates the servo angle based on the given values.
 *
 * @param irPattern
 * 		the current ir board pattern
 * @param currentServoAngle_deg
 * 		the current servo angle in degree
 * @param currentCarVelocity_mps
 * 		the current car velocity in meter per second
 * @return
 * 		the new servo angle
 */
float fuzzy_calculate_servo_angle(unsigned char irPattern, float currentServoAngle_deg, float currentCarVelocity_mps)
{
	NumTypeF4_t result_deg = 0, deviation = 0;

	FuzzyDeviationReturnStatus status = calculate_fuzzy_line_deviation(irPattern, currentServoAngle_deg, &deviation);

	if (status == FUZZY_PATTERN_OK)
	{
//		line_abs_9SI_V_F4_calc(deviation, currentCarVelocity_mps, &result_deg);
		line_abs_9SI_F4_calc(deviation, &result_deg);
	}
	else
	{
		result_deg = currentServoAngle_deg;
	}

	return result_deg;
}

/**
 * Calculates the target velocity of the whole car.
 *
 * @param irPattern
 * 		current ir pattern
 * @param currentServoAngle_deg
 * 		current servo angle in degree
 * @param currentTargetVelocity_mps
 * 		current target velocity in meter per second
 * @return
 * 		the new target velocity
 */
float fuzzy_calculate_target_velocity_mps(unsigned char irPattern, float currentServoAngle_deg, float currentTargetVelocity_mps)
{
	float targetVelocity_mps, absDeviation;

	FuzzyDeviationReturnStatus status = calculate_fuzzy_line_deviation(irPattern, currentServoAngle_deg, &absDeviation);

	absDeviation = fabsf(absDeviation);

	if (status == FUZZY_PATTERN_OK)
	{
		target_speed_F4_calc(absDeviation, &targetVelocity_mps);
	}
	else if (status == FUZZY_PATTERN_TOO_MANY_ONES)
	{
		targetVelocity_mps = currentTargetVelocity_mps;
	}
	else
	{
		targetVelocity_mps = 0;
	}

	return targetVelocity_mps;
}

/**
 * Calculates the input value for the line fuzzy controller.
 *
 * @param irPattern
 * 		the ir Pattern
 * @return
 * 		the fuzzy controller input
 */
FuzzyDeviationReturnStatus calculate_fuzzy_line_deviation(unsigned char irPattern, float currentServoAngle_deg, float* deviation)
{
	FuzzyDeviationReturnStatus status = FUZZY_PATTERN_OK;

	int onesCount = get_ones_count(irPattern);
	char isSingleOnesRow = contains_single_continuous_ones_row(irPattern);
	float value = calculate_ones_mean_value(irPattern) - LINE_INPUT_MAX_NATURAL_VALUE;

	if (onesCount && onesCount <= MAX_ONES_COUNT)
	{
		if ((!isSingleOnesRow) || (fabsf(value) >= FUZZY_INPUT_TRUST_THRESHOLD && sign_f(value) != sign_f(currentServoAngle_deg)))
		{
			value = currentServoAngle_deg > 0 ? LINE_INPUT_MAX_PROCESSABLE_VALUE : -LINE_INPUT_MAX_PROCESSABLE_VALUE;
		}
	}
	else if (!onesCount)
	{
		status = FUZZY_PATTERN_NO_ONES;
	}
	else
	{
		status = FUZZY_PATTERN_TOO_MANY_ONES;
	}

	*deviation = value;

	return status;
}
