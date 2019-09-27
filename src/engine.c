/*
 * engine.c
 *
 *	Hardware Module for engine controlling.
 *
 *  Created on: 15.12.2018
 *      Author: Friedemann Lieberenz
 *
 *  Reviewed by:
 *
 */

#include "engine.h"
#include "enums.h"
#include "iodefine.h"

#define ENGINE_PWM_CYCLE 65536				// 16-bit PWM-Timer max. value, ~10,67ms counting time for one complete cycle
#define MAX_PULSE_WIDTH_PM 1000				// Promille (0 - 1000)

#define ENGINE_SPEED_ZERO 9211 				// compare value for 1.5ms (Nullposition)
#define MAX_PULSE_WIDTH_FORWARD 3070		// difference between 1.5ms and 2ms as value
#define MAX_PULSE_WIDTH_BACKWARD 3080		// difference between 1ms and 1.5ms as value
#define ENGINE_BREAK 6141					// value for engine to stop completely (1ms)
#define ENGINE_SPEED_MAX 12281				// compare value for 2ms, full speed (for testing purposes only)

signed char g_engine_current_state;			// '1'=FORWARD, '-1'=BACKWARD, '0'=BREAK OR FREERUN

/**
 * Initializes the engine module. Initial pulse width: 9211(1.5ms), initial mode: FORWARD.
 * @note
 * 		Must be called first, otherwise the engine module won't work properly.
 */
void engine_initialize()
{
	SYSTEM.MSTPCRA.BIT.MSTPA7 = 0;				// Enable General-PWM-Timer module (GPT)

	//GPT1 - Left Wheel
	PORT7.DDR.BIT.B2 = 1;						// P72: PWM output (GTIOC1A)
	PORT7.ICR.BIT.B2 = 0;						// Disable Input Buffer on P72
	GPT.GTSTR.BIT.CST1 = 0;						// GPT1 stop counting (only during init)
	GPT1.GTIOR.BIT.GTIOA = 9;					// Low output on Compare Match, High output at cycle end
	GPT1.GTCR.BIT.MD = 4;						// Set Triangle-Wave PWM Mode 1
	GPT1.GTCR.BIT.TPCS = 3;						// Clk/8 prescaler (6.14MHz)
	GPT1.GTBER.BIT.CCRA = 0;					// No Buffer transfer at counter overflow
	GPT1.GTUDC.BIT.UD = 1;						// Count up
	GPT1.GTPR = 0xFFFF;							// Set max count value
	GPT1.GTONCR.BIT.OAE = 1;					// Enable Output Pin GTIOC1A
	GPT1.GTCNT = 0;								// initial count value = 0
	GPT1.GTCCRA = ENGINE_SPEED_ZERO; 			// set initial compare value (Nullposition)

	//GPT2 - Right Wheel
	PORT7.DDR.BIT.B3 = 1;						// P73: PWM output (GTIOC2A)
	PORT7.ICR.BIT.B3 = 0;						// Disable Input Buffer on P73
	GPT.GTSTR.BIT.CST2 = 0;						// GPT2 stop counting (only during init)
	GPT2.GTIOR.BIT.GTIOA = 9;					// Low output on Compare Match, High output at cycle end
	GPT2.GTCR.BIT.MD = 4;						// Set Triangle-Wave PWM Mode 1
	GPT2.GTCR.BIT.TPCS = 3;						// Clk/8 prescaler (6.14MHz)
	GPT2.GTBER.BIT.CCRA = 0;					// No Buffer transfer at counter overflow
	GPT2.GTUDC.BIT.UD = 1;						// Count up
	GPT2.GTPR = 0xFFFF;							// Set max count value
	GPT2.GTONCR.BIT.OAE = 1;					// Enable Output Pin GTIOC2A
	GPT2.GTCNT = 0;								// initial count value = 0
	GPT2.GTCCRA = ENGINE_SPEED_ZERO; 			// set initial compare value (Nullposition)

	GPT.GTSTR.BIT.CST1 = 1;						// GPT1 start counting
	GPT.GTSTR.BIT.CST2 = 1;						// GPT2 start counting

	g_engine_current_state = 1;					//initial mode
	//Basically a Timer of 2 seconds to get the controller to start running.
	for (int i=1; i<=200;i++){
		while (!GPT1.GTST.BIT.TCFPO);
		GPT1.GTST.BIT.TCFPO = 0;
	}
}

/**
 * Sets the pulse width in promille of the specified engine PWM signal.
 *
 * @param side
 * 		determines the engine
 * @param pulse_width
 * 		the pulse width in promille (0 - 1000)
 */
void engine_set_pulse_width_pm(enum Side side, unsigned int pulse_width)
{
	//restrict pulse width to a maximum of 1000 promille
	if (pulse_width > MAX_PULSE_WIDTH_PM)
		pulse_width = MAX_PULSE_WIDTH_PM;

	//calculate pulse width (from promille to value)
	if (g_engine_current_state == 1)		//current state is FORWARD
		pulse_width = ((MAX_PULSE_WIDTH_FORWARD * pulse_width) /MAX_PULSE_WIDTH_PM) + ENGINE_SPEED_ZERO;

	else if (g_engine_current_state == -1)	//current state is BACKWARD
		pulse_width = ENGINE_SPEED_ZERO - (MAX_PULSE_WIDTH_BACKWARD * pulse_width /MAX_PULSE_WIDTH_PM);

	//set the pulse width
	if (side == LEFT)
		GPT1.GTCCRA = pulse_width;
	else if (side == RIGHT)
		GPT2.GTCCRA = pulse_width;
}

/**
 * Returns the current pulse width of the specified engine PWM signal in promille.
 *
 * @param side
 * 		determines the engine
 * @return
 * 		the pulse width in promille (0 - 1000)
 */
unsigned int engine_get_pulse_width_pm(enum Side side)
{
	unsigned int pulse_width = 0;

	//Read current pulse width
	if (side == LEFT)
		pulse_width = GPT1.GTCCRA;
	else if (side == RIGHT)
		pulse_width = GPT2.GTCCRA;

	//calculate pulse width (value to promille)
	if (pulse_width > ENGINE_SPEED_ZERO){			//current state = FORWARD
		pulse_width = pulse_width - ENGINE_SPEED_ZERO;
		pulse_width = (pulse_width * MAX_PULSE_WIDTH_PM) / MAX_PULSE_WIDTH_FORWARD;
	}
	else if (pulse_width < ENGINE_SPEED_ZERO){		//current state = BACKWARD
		pulse_width = ENGINE_SPEED_ZERO - pulse_width;
		pulse_width = (pulse_width * MAX_PULSE_WIDTH_PM) / MAX_PULSE_WIDTH_BACKWARD;
	}
	else
		pulse_width = 0;

	return pulse_width;
}

/**
 * Sets the operation mode of the specified engine.
 *
 * @param side
 * 		determines the engine
 * @param mode
 *  	the operation mode
 */
void engine_set_mode(enum Side side, enum OperationMode mode)
{
	//FORWARD
	if (mode == FORWARD_FREERUN || mode == FORWARD_BREAK)
		g_engine_current_state = 1;

	//BACKWARD
	else if (mode == BACKWARD_FREERUN || mode == BACKWARD_BREAK)
		g_engine_current_state = -1;

	//BREAK
	else if (mode == BREAK){
		//set the pulse width to 1ms	-> evtl. hier ändern, wenn Bremsen nicht funktioniert (Funktionsweise Bremse, ein Zyklus muss es sein)
		GPT1.GTCCRA = ENGINE_BREAK;
		GPT2.GTCCRA = ENGINE_BREAK;
		g_engine_current_state =0;
	}
	else if (mode == FREERUN){
		//set the pulse width to 1.5ms
		GPT1.GTCCRA = ENGINE_SPEED_ZERO;
		GPT2.GTCCRA = ENGINE_SPEED_ZERO;
		g_engine_current_state =0;
	}
}

/**
 * Returns the operation mode of the specified engine.
 *
 * @param side
 * 		determines the engine
 * @return
 * 		the operation mode
 */
enum OperationMode engine_get_mode(enum Side side)
{
	enum OperationMode mode = BREAK;

	//FORWARD
	if (g_engine_current_state == 1)
		mode = FORWARD_FREERUN;
	//BACKWARD
	else if (g_engine_current_state == -1)
		mode = BACKWARD_FREERUN;
	//BREAK
	else if (g_engine_current_state == 0 && GPT1.GTCCRA == ENGINE_BREAK && GPT2.GTCCRA == ENGINE_BREAK)
		mode = BREAK;
	//FREERUN
	else if (GPT1.GTCCRA == ENGINE_SPEED_ZERO && GPT2.GTCCRA == ENGINE_SPEED_ZERO)
		mode = FREERUN;

	return mode;
}
