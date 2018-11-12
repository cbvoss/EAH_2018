/*
 * math_tools.c
 *
 *  Created on: Jan 12, 2018
 *      Author: Tobias Hupel
 */

#include "math_tools.h"

/**
 * Calculates the percentage from the given value to the other given value.
 *
 * @param from_value
 *		percentage to calculate
 * @param of_value
 * 		reference value
 * @return
 * 		the percentage
 */
float percentage_f(float from_value, float of_value)
{
	return from_value / of_value * MAX_NORMALIZED_PERCENTAGE;
}

/**
 * Converts an angle from degrees to radiants.
 *
 * @param angle_rad
 * 		angle in radiants
 * @return
 * 		angle in degree
 */
float angle_rad_to_deg_f(float angle_rad)
{
	return angle_rad / PI_RAD_F * PI_DEG_F;
}

/**
 * Converts an angle from degrees to radiants.
 *
 * @param angle_deg
 * 		angle in degree
 * @return
 * 		angle in radiants
 */
float angle_deg_to_rad_f(float angle_deg)
{
	return angle_deg / PI_DEG_F * PI_RAD_F;
}

int sign_f(float value)
{
	return value > 0 ? 1 : value < 0 ? -1 : 0;
}
