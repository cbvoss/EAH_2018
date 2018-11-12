/*
 * servo.c
 *
 *  Created on: 23.11.2016
 *      Author: Florian Schaedlich
 *
 *  Reviewed by:
 *  			Franz Luebke
 *  			Florian Schaedlich
 */

#include "servo.h"
#include "iodefine.h"
#include "config_car.h"

#define MAX_PULSE_WIDTH_PM 1000
#define MIN_PULSE_WIDTH_PM -1000
#define SERVO_PWM_CYCLE 65536

int g_servo_center = 61000;				// initialize servo with center value: 15%
int g_servo_max_right = 60000;			// initialize servo left: 5%
int g_servo_max_left = 62000;			// initialize servo right: 25%

int g_current_position_pm = 0;			// current servo position in relation to servo center
float g_current_position_deg = 0;

void set_raw_unchecked_pulse_width_pm(int pulse_wisth_pm);

/**
 * Initializes the servo module. Initial pulse width: 0
 * @note
 * 		Must be called first, so the servo module could work
 */
void servo_initialize()
{
	PORT7.DDR.BYTE |= 0x0002;					// P71: Servo-PWM output
	PORT7.ICR.BIT.B1 = 0;						// Disable Input Buffer

	SYSTEM.MSTPCRA.BIT.MSTPA7 = 0;				// Enable GPT module

	GPT.GTSTR.BIT.CST0 = 0;						// b0: GPT0 stop counting
	GPT0.GTCR.BIT.MD = 4;						// Set Triangle-Wave PWM Mode 1
	GPT0.GTCR.BIT.TPCS = 3;						// Iclk/8
	GPT0.GTPR = 0xFFFF;							// Set max count value
	GPT0.GTCNT = 0x0000;						// initial count value
	GPT0.GTIOR.BIT.GTIOA = 3;					// Toggle on Compare Match
	GPT0.GTONCR.BIT.OAE = 1;					// Enable Output Pin GTIOC
	GPT0.GTBER.BIT.CCRA = 1;					// Buffer transfer at counter overflow/underflow, GTCCRA single buffer (GTCCRC) enabled
	GPT0.GTCCRA = GPT0.GTCCRC = g_servo_center; // set compare match to 15 percent from max PWM
	GPT.GTSTR.BIT.CST0 = 1;						// start counting on GPT0
}

/**
 * Set borders for Servo Register and servo center and change servo position to new center
 * Important: register value
 * @param center
 * @param right: lowest register value
 */
void servo_set_borders(unsigned int left, unsigned int center, unsigned int right)
{
	g_servo_max_left = left;
	g_servo_max_right = right;
	g_servo_center = center;

	GPT0.GTCCRC = g_servo_center;

	g_current_position_pm = 0;
	g_current_position_deg = 0;
}

/**
 * Get the current position in permille in relation to center position
 * left: positive
 * right: negative
 * center: zero
 * (mathematical positive sense)
 * @return pulse_width_pm
 */
int servo_get_position_pm(void)
{
	return g_current_position_pm;
}

/**
 * Returns the servo position in degree.
 *
 * @return
 * 		the servo position in degree
 */
float servo_get_position_angle_deg(void)
{
	return g_current_position_deg;
}

/**
 * Sets the position of the servo in degree.
 *
 * @param angle_deg
 * 		the servo position in degree
 */
void servo_set_position_angle(float angle_deg)
{
	g_current_position_deg = angle_deg;

	if (g_current_position_deg > MAX_SERVO_ANGLE_LEFT_ABS_DEG)
		g_current_position_deg = MAX_SERVO_ANGLE_LEFT_ABS_DEG;
	else if (g_current_position_deg < -MAX_SERVO_ANGLE_RIGHT_ABS_DEG)
		g_current_position_deg = -MAX_SERVO_ANGLE_RIGHT_ABS_DEG;

	if (g_current_position_deg > 0)
		g_current_position_pm = g_current_position_deg / MAX_SERVO_ANGLE_LEFT_ABS_DEG * MAX_PULSE_WIDTH_PM;
	else if (g_current_position_deg < 0)
		g_current_position_pm = g_current_position_deg / -MAX_SERVO_ANGLE_RIGHT_ABS_DEG * MIN_PULSE_WIDTH_PM;
	else
		g_current_position_pm = 0;

	set_raw_unchecked_pulse_width_pm(g_current_position_pm);
}


/**
 * Set the position in permille from left(max) or right(max) in relation to center position
 * left: positive
 * right: negative
 * center: zero
 * @param pulse_width
 */
void servo_set_position_pm(int position_pm)
{
	g_current_position_pm = position_pm;

	if (g_current_position_pm > MAX_PULSE_WIDTH_PM)
		g_current_position_pm = MAX_PULSE_WIDTH_PM;
	if (g_current_position_pm < MIN_PULSE_WIDTH_PM)
		g_current_position_pm = MIN_PULSE_WIDTH_PM;

	if (g_current_position_pm > 0)
		g_current_position_deg = ((float)g_current_position_pm) / MAX_PULSE_WIDTH_PM * MAX_SERVO_ANGLE_LEFT_ABS_DEG;
	else if (g_current_position_pm < 0)
		g_current_position_deg = ((float)g_current_position_pm) / MIN_PULSE_WIDTH_PM * -MAX_SERVO_ANGLE_RIGHT_ABS_DEG;
	else
		g_current_position_deg = 0;

	set_raw_unchecked_pulse_width_pm(g_current_position_pm);
}

/**
 * Writes the pulse width into the regarding registers.
 *
 * @param pulse_wisth_pm
 * 		the pulse width in permille
 */
void set_raw_unchecked_pulse_width_pm(int pulse_wisth_pm)
{
	if (pulse_wisth_pm == 0)
		GPT0.GTCCRC = g_servo_center;
	else if (pulse_wisth_pm > 0)
		GPT0.GTCCRC = g_servo_center + (((g_servo_center - g_servo_max_left) * pulse_wisth_pm) / MAX_PULSE_WIDTH_PM);
	else
		GPT0.GTCCRC = g_servo_center + (((g_servo_max_right - g_servo_center) * pulse_wisth_pm) / MAX_PULSE_WIDTH_PM);
}

