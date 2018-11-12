/*
 * switches.c
 *
 *  Created on: 06.03.2017
 *      Author: Eric Elsner
 */

#include "switches.h"

/**
 * Initializes the module.
 */
void switches_initialize()
{
	//IOPORT.PFMPOE.BIT.POE0E = 1;	// enable P70 (motor drive board switch sw1)
	PORT7.ICR.BIT.B0 = 1;
	PORT7.DDR.BIT.B0 = 0; //input
}

/**
 * Returns the state of the switch located on the motor drive board (sw1).
 * @return
 * 	"1" if the switch is pressed. "0" if not.
 */
char switches_get_MotorDriveBoardSwitch()
{
	char c;
	c = (PORT7.PORT.BIT.B0 == 0) ? 1:0;

	return c;
}
