/*
 * servo.h
 *
 *  Created on: 23.11.2016
 *      Author: Florian Schaedlich
 *
 *  Reviewed by:
 *  			Franz Luebke
 *  			Florian Schaedlich
 */

#ifndef SERVO_H_
#define SERVO_H_

void servo_initialize();



void servo_set_borders(unsigned int left, unsigned int center, unsigned int right);

int servo_get_position_pm(void);

void servo_set_position_pm(int position_pm);

float servo_get_position_angle_deg(void);

void servo_set_position_angle(float angle);

#endif /* SERVO_H_ */
