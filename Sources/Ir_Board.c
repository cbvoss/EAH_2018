/*
 * Ir_Board.c
 *
 *  Created on: 05.11.2016
 *      Author: Eric Elsner
 */

#include "iodefine.h"

void ir_initialize(){
	PORT4.ICR.BYTE=0xff;
}

/**
 * Returns the pattern of the 8 sensors.
 * When watching into forward direction of the car
 * the least significant bit is the most right one.
 *
 * @return
 * A return value of '0' means 'dark' while a '1' means bright.
 */
unsigned char ir_get_pattern(){
	 unsigned char sensor;

	 sensor  = ~PORT4.PORT.BYTE;

	 return sensor;
}

/**
 * Returns the state of one specific sensor element.
 *
 * @param elementIndex
 * 	0...7 -> Element 0 is the most right one when looking into forward direction of the car.
 * @return
 * 	0 (dark) or 1 (bright)
 */
char ir_get_value(char elementIndex){
	unsigned char mask;
	char result;

	mask = 1 << elementIndex;
	result = ir_get_pattern();
	result &= mask;
	result = result == 0 ? 0:1;

	return result;
}

/**
 * Determines whether the front sensor detects something.
 *
 * @return
 * 	'1' means 'object detected'. '0' means 'no object detected'
 */
char ir_get_front_sensor()
{
	// front sensor and most right sensor are coupled by a logical OR
	char result = ir_get_value(0);
	return result;
}
