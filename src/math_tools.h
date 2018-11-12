/*
 * math_tools.h
 *
 *  Created on: Jan 12, 2018
 *      Author: Tobias Hupel
 */

#ifndef TOOLS_MATH_TOOLS_H_
#define TOOLS_MATH_TOOLS_H_

#define PI_RAD_F 3.141592654f
#define PI_DEG_F 180.0f

#define MAX_NORMALIZED_PERCENTAGE 100.0f

float percentage_f(float from_value, float of_value);

float angle_rad_to_deg_f(float angle_rad);

float angle_deg_to_rad_f(float angle_deg);

int sign_f(float value);

#endif /* TOOLS_MATH_TOOLS_H_ */
