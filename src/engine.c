/*
 * engine.c
 *
 *	Hardware Module for engine controlling.
 *
 *  Created on: 6 Nov 2016
 *      Author: Tobias Hupel
 *
 *  Reviewed by:
 *  			Florian Schaedlich
 *  			Tobias Hupel
 */

#include "engine.h"

#include "enums.h"
#include "iodefine.h"

#define ENGINE_PWM_CYCLE 65536				// 0xffff + 1, 2^16, Motor PWM period length (10.49ms at ILCK / 8 and ICLK = 50MHz)
#define MAX_PULSE_WIDTH_PM 1000				// Promille (0 - 1000)

#define LEFT_PWM_PORT_OUTPUT_MASK 0x04		// 0b00000100, GTIOC1A P72: PWM left motor
#define RIGHT_PWM_PORT_OUTPUT_MASK 0x08		// 0b00001000, GTIOC2A P73: PWM right motor

#define LEFT_FORWARD_BACKWARD_MASK 0x10		// 0b00010000, P74: forward (0) and backward (1) left motor
#define RIGHT_FORWARD_BACKWARD_MASK 0x20	// 0b00100000, P75: forward (0) and backward (1) right motor

#define LEFT_BREAK_FREERUN_MASK 0x01		// 0b00000001, P10: break (0) and freerun (1) left motor, LED2 motor drive board
#define RIGHT_BREAK_FREERUN_MASK 0x40		// 0b01000000, P76: break (0) and freerun (1) right motor, LED3 motor drive board

#define PWM_PIN_OUTPUT_ENABLE_MASK 0x4000	// GPTn.GTONCR b14: GTIOC1A pin output enable (1) and disable (0)

unsigned char get_forward_backward_mask(enum Side side)
{
	return side == RIGHT ?
	RIGHT_FORWARD_BACKWARD_MASK :
							LEFT_FORWARD_BACKWARD_MASK;
}

unsigned char get_break_freerun_mask(enum Side side)
{
	return side == RIGHT ? RIGHT_BREAK_FREERUN_MASK : LEFT_BREAK_FREERUN_MASK;
}

unsigned char get_pwm_port_output_mask(enum Side side)
{
	return side == RIGHT ?
	RIGHT_PWM_PORT_OUTPUT_MASK :
							LEFT_PWM_PORT_OUTPUT_MASK;
}

/**
 * Initializes the engine module. Initial pulse width: 0, initial mode: BREAK.
 * @note
 * 		Must be called first, otherwise the engine module won't work properly.
 */
void engine_initialize()
{
	SYSTEM.MSTPCRA.BIT.MSTPA7 = 0;				// Enable GPT module

	PORT1.DDR.BYTE |= LEFT_BREAK_FREERUN_MASK;

	PORT7.DDR.BYTE |= LEFT_PWM_PORT_OUTPUT_MASK | RIGHT_PWM_PORT_OUTPUT_MASK | LEFT_FORWARD_BACKWARD_MASK | RIGHT_FORWARD_BACKWARD_MASK
			| RIGHT_BREAK_FREERUN_MASK;

	GPT.GTSTR.WORD &= ~0x0006;						// b1: GPT1 stop counting
													// b2: GPT2 stop counting

	GPT1.GTIOR.WORD = GPT2.GTIOR.WORD = 0x0009;		// GTIOC1A, GTIOC2A: Initial Value: "0", value at cycle end: "1", value at GTCCRA compare match: "0"

	GPT1.GTCR.WORD = GPT2.GTCR.WORD = 0x0300;		// Saw-Wave PWM Mode, Prescaler: ICLK / 8;

	GPT1.GTBER.WORD = GPT2.GTBER.WORD = 0x0301;		// Buffer transfer at counter overflow/underflow, GTCCRA single buffer (GTCCRC) enabled

	GPT1.GTPR = GPT2.GTPR = 0xffff;					// max Count Value (PWM period length - 1)

	GPT1.GTUDC.WORD = GPT2.GTUDC.WORD = 0x0001;		// count upwards

	GPT1.GTCNT = GPT2.GTCNT = 0x0000;				// initial count value

	GPT1.GTCCRA = GPT1.GTCCRC = 0x0000;				// set compare match to 0
	GPT2.GTCCRA = GPT2.GTCCRC = 0x0000;				// set compare match to 0

	GPT.GTSTR.WORD |= 0x0006;						// start counting on GPT1 and GPT2
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
	if (pulse_width > MAX_PULSE_WIDTH_PM)
		pulse_width = MAX_PULSE_WIDTH_PM;

	pulse_width = (ENGINE_PWM_CYCLE - 1) * pulse_width / MAX_PULSE_WIDTH_PM;

	if (side == LEFT)
		GPT1.GTCCRC = pulse_width;
	else if (side == RIGHT)
		GPT2.GTCCRC = pulse_width;
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

	if (side == LEFT)
		pulse_width = GPT1.GTCCRC;
	else if (side == RIGHT)
		pulse_width = GPT2.GTCCRC;

	pulse_width = pulse_width * MAX_PULSE_WIDTH_PM / (ENGINE_PWM_CYCLE - 1);

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
	unsigned char forward_backward_mask = get_forward_backward_mask(side);
	unsigned char pwm_port_output_mask = get_pwm_port_output_mask(side);

	// FORWARD or BACKWARD -> enable PWM Pin Output
	if (mode == FORWARD_FREERUN || mode == BACKWARD_FREERUN || mode == FORWARD_BREAK || mode == BACKWARD_BREAK)
	{
		// enable PWM Pin Output
		if (side == LEFT)
			GPT1.GTONCR.WORD |= PWM_PIN_OUTPUT_ENABLE_MASK;
		else if (side == RIGHT)
			GPT2.GTONCR.WORD |= PWM_PIN_OUTPUT_ENABLE_MASK;
	}
	// BREAK or FREERUN -> disable PWM Pin Output
	else if (mode == BREAK || mode == FREERUN)
	{
		// disable PWM pin output
		if (side == LEFT)
			GPT1.GTONCR.WORD &= ~PWM_PIN_OUTPUT_ENABLE_MASK;
		else if (side == RIGHT)
			GPT2.GTONCR.WORD &= ~PWM_PIN_OUTPUT_ENABLE_MASK;

		// set PWM output pin to "0"
		PORT7.DR.BYTE &= ~pwm_port_output_mask;
	}

	// set desired mode (FORWARD or BACKWARD)
	if (mode == FORWARD_FREERUN || mode == FORWARD_BREAK)
		PORT7.DR.BYTE &= ~forward_backward_mask;
	else if (mode == BACKWARD_FREERUN || mode == BACKWARD_BREAK)
		PORT7.DR.BYTE |= forward_backward_mask;

	// set desired mode (BREAK or FREERUN)
	if (mode == BREAK || mode == FORWARD_BREAK || mode == BACKWARD_BREAK)
		if (side == LEFT)
			PORT1.DR.BYTE &= ~LEFT_BREAK_FREERUN_MASK;
		else
			PORT7.DR.BYTE &= ~RIGHT_BREAK_FREERUN_MASK;
	else if (mode == FREERUN || mode == FORWARD_FREERUN || mode == BACKWARD_FREERUN)
		if (side == LEFT)
			PORT1.DR.BYTE |= LEFT_BREAK_FREERUN_MASK;
		else
			PORT7.DR.BYTE |= RIGHT_BREAK_FREERUN_MASK;
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
	unsigned char forward_backward_mask = get_forward_backward_mask(side);

	enum OperationMode mode = BREAK;

	if ((side == LEFT && (GPT1.GTONCR.WORD & PWM_PIN_OUTPUT_ENABLE_MASK)) || (side == RIGHT && (GPT2.GTONCR.WORD & PWM_PIN_OUTPUT_ENABLE_MASK)))
	{
		if ((side == LEFT && (PORT1.DR.BYTE & LEFT_BREAK_FREERUN_MASK)) || (side == RIGHT && (PORT7.DR.BYTE & RIGHT_BREAK_FREERUN_MASK)))
		{
			if (PORT7.DR.BYTE & forward_backward_mask)
				mode = BACKWARD_FREERUN;
			else
				mode = FORWARD_FREERUN;
		}
		else
		{
			if (PORT7.DR.BYTE & forward_backward_mask)
				mode = BACKWARD_BREAK;
			else
				mode = FORWARD_BREAK;
		}
	}
	else
	{
		if ((side == LEFT && (PORT1.DR.BYTE & LEFT_BREAK_FREERUN_MASK)) || (side == RIGHT && (PORT7.DR.BYTE & RIGHT_BREAK_FREERUN_MASK)))
			mode = FREERUN;
		else
			mode = BREAK;
	}

	return mode;
}
