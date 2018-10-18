/*
 * differential.h
 *
 *  Created on: 21.09.2017
 *      Author: Simon
 */

#ifndef DIFFERENTIAL_H_
#define DIFFERENTIAL_H_

void diff_calculate(float W_m, float T_m, float servoAngle_rad, float targetVelocity_mps, float * innerVelocity_mps, float * outerVelocity_mps);

void diff_reverse_calculate(float W_m, float T_m, float innerVelocity_mps, float outerVelocity_mps, float* servoAngleAbs_rad);

#endif /* DIFFERENTIAL_H_ */
