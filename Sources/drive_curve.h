/*
 * drive_curve.h
 *
 *  Created on: 07.01.2017
 *      Author: franz_2
 */

#ifndef DRIVE_CURVE_H_
#define DRIVE_CURVE_H_
#include "enums.h"
#include "config_car.h"
enum curve_size {
	R600,
	R450,
	Txxx
};
struct Memory{
	int buffer[64];
	int sum;
	unsigned char w_ptr;
};


void drive_curve_set_active(char active);
void drive_curve_set_max_speed(float speed);
void drive_curve_initialize();
void drive_curve_update();
void drive_curve_update_fixed();
int drive_curve_ir_deviation(unsigned char ir_pattern);
void drive_curve_reset();
void drive_curve_manipulate_memory(int mem);
char drive_curve_good_position();
float drive_curve_servo_regulator();
#ifdef DEBUG_data_log
float debug_get_wheel_reg_val();
float drive_curve_get_I();
int drive_curve_get_memory_debug();
#endif

#ifdef DEBUG_drive_curve

void drive_curve_set_regulator(float V, float I, float D);
void drive_curve_set_curve(enum curve_size size, enum Side side);
void drive_curve_kalib_curve(enum curve_size size, int pm_winkel);
void drive_curve_set_servo_regulator(float V, float I, float D);
#endif

#endif /* DRIVE_CURVE_H_ */
