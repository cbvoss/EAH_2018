/*
 * regulator.h
 *
 *	Module for regulation purposes.
 *
 *  Created on: Dec 7, 2016
 *      Author: Tobias Hupel
 */

#ifndef REGULATOR_H_
#define REGULATOR_H_

/**
 * Struct defining a PID - Regulator.
 */
struct Regulator {
	float v, ti_inverse, td, time_interval, last_deviation, deviation_sum, windup_border;
};

void regulator_new(struct Regulator *regulator, float v, float ti_inverse, float td, float time_interval);

void regulator_set_V(struct Regulator *regulator, float v);
void regulator_set_ti_inverse(struct Regulator *regulator, float ti_inverse);
void regulator_set_td(struct Regulator *regulator, float td);

float regulator_calculate_value(struct Regulator *regulator, float deviation);
void regulator_set_windup(struct Regulator *regulator, float windup_border);

#endif /* REGULATOR_H_ */
