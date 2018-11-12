/*
 * fuzzy.h
 *
 *  Created on: Dec 7, 2017
 *      Author: Tobias Hupel
 */

#ifndef FUZZY_FUZZY_H_
#define FUZZY_FUZZY_H_

typedef enum
{
	FUZZY_PATTERN_TOO_MANY_ONES, FUZZY_PATTERN_NO_ONES, FUZZY_PATTERN_OK
}
FuzzyDeviationReturnStatus;

void fuzzy_initialize();

void fuzzy_update();

float fuzzy_calculate_servo_angle(unsigned char irPattern, float currentServoAngle_deg, float currentCarVelocity_mps);

float fuzzy_calculate_target_velocity_mps(unsigned char irPattern, float currentServoAngle_deg, float currentTargetVelocity_mps);

#endif /* FUZZY_FUZZY_H_ */
