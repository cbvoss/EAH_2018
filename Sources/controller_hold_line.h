/*
 * controller_hold_line.h
 *
 *  Created on: 21.12.2016
 *      Author: franz luebke
 *
 *      use the IR Analog sensors and the both rear wheels to hold the center of the path.
 */

#ifndef CONTROLLER_HOLD_LINE_H_
#define CONTROLLER_HOLD_LINE_H_

#include "typedefine.h"

void hold_line_initalize(void);
void hold_line_fixed_update(void);
void hold_line_update(void);
void hold_line_set_max_speed(float speed);
void hold_line_set_active(char active);
char hold_line_good_position(char Epsilon);
void hold_line_reset();

#ifdef DEBUG
float hold_line_get_regulator_value();
float hold_line_get_error(void);

int hold_line_get_servo_regulator_value(void);
#endif
#endif /* CONTROLLER_HOLD_LINE_H_ */
