/*
 * differential.c
 *
 *  Created on: 21.09.2017
 *      Author: Simon
 */

#include <math.h>

/**
 * Calculates the inner and outer wheel velocity depending on the servo angle equivalent to a mechanic differential.
 *
 * @param W_m
 * 		the cars wheelbase (Achsenabstand vorne zu hinten)
 * @param T_m
 * 		the cars tread (Abstand zwischen 2 Rädern einer Achse)
 * @param servoAngleAbs_rad
 * 		the current absoulte servo angle
 * @param targetVelocity_mps
 * 		the cars target velocity
 * @param innerVelocity_mps
 * 		the cars resulting inner wheel velocity
 * @param outerVelocity_mps
 * 		the cars resulting outer wheel velocity
 */
void diff_calculate(float W_m, float T_m, float servoAngleAbs_rad, float targetVelocity_mps, float * innerVelocity_mps, float * outerVelocity_mps)
{
	float r2 = W_m / tanf(servoAngleAbs_rad);
	float r1 = r2 - (T_m / 2);
	float r3 = r2 + (T_m / 2);

	*innerVelocity_mps = r1 / r2 * targetVelocity_mps;
	*outerVelocity_mps = r3 / r2 * targetVelocity_mps;
}

/**
 * Calculates the absolute servo angle depending on the cars inner and outer wheel velocities. Equivalent to an "reverse" differential.
 *
 * @param W_m
 * 		the cars wheelbase
 * @param T_m
 * 		the cars tread
 * @param innerVelocity_mps
 * 		the cars inner wheel velocity
 * @param outerVelocity_mps
 * 		the cars outer wheel velocity
 * @param servoAngleAbs_rad
 * 		the cars resulting absolute servo angle
 */
void diff_reverse_calculate(float W_m, float T_m, float innerVelocity_mps, float outerVelocity_mps, float* servoAngleAbs_rad)
{
	float r2 = T_m / 2 * (1 + innerVelocity_mps / outerVelocity_mps) / (1 - innerVelocity_mps / outerVelocity_mps);

	*servoAngleAbs_rad = atanf(W_m / r2);
}


