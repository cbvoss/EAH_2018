/*
 * regulator.c
 *
 *	Module for regulation purposes.
 *
 *  Created on: Dec 7, 2016
 *      Author: Tobias Hupel
 */

#include "regulator.h"
/**
 *	Initializes a new regulator with the specified properties.
 *
 * @param regulator
 * 		the address to the regulator which should be initialized
 * @param v
 * 		the amplification
 * @param ti_inverse
 * 		Ti inverse
 * @param td
 * 		Td inverse
 * @param time_interval
 * 		the time between 2 calculation steps
 */
void regulator_new(struct Regulator *regulator, float v, float ti_inverse, float td, float time_interval)
{
	regulator->v = v;
	regulator->ti_inverse = ti_inverse;
	regulator->td = td;
	regulator->time_interval = time_interval;
	regulator->last_deviation = 0.0f;
	regulator->deviation_sum = 0.0f;
	regulator->windup_border=0;
}

/**
 * Overwrite the current V-parameter with a given value
 * @param regulator
 * 		the address of the regulator which should be changed
 * @param v
 * 		the new amplification
 */
void regulator_set_V(struct Regulator *regulator, float v){
	regulator->v = v;
}

/**
 * Overwrite the current ti_inverse-parameter with a given value
 * @param regulator
 * 		the address of the regulator which should be changed
 * @param ti_inverse
 * 		the new Ti-inverse
 */
void regulator_set_ti_inverse(struct Regulator *regulator, float ti_inverse){
	regulator->ti_inverse = ti_inverse;
}

/**
 * Overwrite the current td-parameter with a given value
 * @param regulator
 * 		the address of the regulator which should be changed
 * @param td_inverse
 * 		the new td-inverse
 */
void regulator_set_td(struct Regulator *regulator, float td){
	regulator->td = td;
}

/**
 * Calculate the i part from the regulator.
 *  It use optional the windup border.
 * @param regulator
 * @return i-part
 */
float regulator_ti_inverse(struct Regulator *regulator){
	float i =(regulator->ti_inverse * regulator->time_interval * regulator->deviation_sum);
	if(regulator->windup_border>0.0f){
		regulator->deviation_sum=(regulator->deviation_sum>regulator->windup_border)?regulator->windup_border:regulator->deviation_sum;
		regulator->deviation_sum=(regulator->deviation_sum<-1.0f*regulator->windup_border)?-1.0f*regulator->windup_border:regulator->deviation_sum;
	}
	return i;
}

/**
 * Calculates the new output value of the specified PID - Regulator.
 *
 * @note
 *  	Automatically takes care of the last deviation and the deviation sum.
 *
 * @param regulator
 * 		the regulator to use
 * @param deviation
 * 		current regulation deviation
 * @return
 * 		the new regulator output value
 */
float regulator_calculate_value(struct Regulator *regulator, float deviation)
{
	float value;
	//regulator->deviation_sum += deviation;
	regulator->deviation_sum += (deviation + regulator->last_deviation)/2;

	value = regulator->v
			* (deviation + regulator_ti_inverse(regulator)
			+ (regulator->td * (deviation - regulator->last_deviation) / regulator->time_interval));

	regulator->last_deviation = deviation;

	return value;
}
/**
 * set the windup border from the regulator
 * @param regulator
 * @param windup_border
 */
void regulator_set_windup(struct Regulator *regulator, float windup_border){
	regulator->windup_border=(windup_border/(regulator->ti_inverse * regulator->time_interval))/regulator->v;
}

