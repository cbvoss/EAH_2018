/*
 * debug_routines.c
 *
 *  Created on: Jan 15, 2018
 *      Author: Tobias Hupel
 */

#include "global_clock.h"
#include "drive.h"
#include "servo.h"
#include "serial_blue.h"
#include "enums.h"
#include "tachometer.h"
#include <stdio.h>
#include "engine.h"
#include "opto_interruptor.h"

void debug_drive_regulator(char startSign, int time1, int time2, int stoptime, int inTime,
		float initialV_mps, float secondV_mps, float thirdV_mps, char ubreak, float threshold)
{
	static struct timer inter, t1, t2, tend;
	static char started = 0, t1started = 0, t2started = 0, stop = 0;
	static unsigned int counter = 0;

	if (!started && !stop && serial_blue_pull_receive_buffer() == startSign)
	{
		servo_set_position_pm(0);

		drive_accelerate_to_differential_advanced(initialV_mps, ubreak, threshold);

		global_clock_set_timer(&t1, time1);
		global_clock_set_timer(&t2, time2);
		global_clock_set_timer(&tend, stoptime);
		global_clock_set_timer(&inter, inTime);

		serial_blue_write_string("count;servo;list;lsoll;rist;rsoll\n");

		started = 1;
	}

	if (started && !stop && !t1started && global_clock_timer(&t1))
	{
		drive_accelerate_to_differential_advanced(secondV_mps, ubreak, threshold);

		t1started = 1;
	}

	if (started && !stop && !t2started && global_clock_timer(&t2))
	{
		drive_accelerate_to_differential_advanced(thirdV_mps, ubreak, threshold);

		t2started = 1;
	}

	if (started && !stop && global_clock_timer(&tend))
	{
		drive_accelerate_to_differential_advanced(0, 1, 0);

		stop = 1;
	}

	if (started && !stop && global_clock_timer(&inter))
	{
		global_clock_reset_timer(&inter);

    	char buff[64];
    	buff[63] = 0;

    	float lv = tachometer_get_velocity_mps(BACK_LEFT), rv = tachometer_get_velocity_mps(BACK_RIGHT);
    	float lvs = drive_get_wheel_target_velocity_mps(LEFT);
    	float rvs = drive_get_wheel_target_velocity_mps(RIGHT);
    	float angle = servo_get_position_angle_deg();

    	sprintf(buff, "%f;%f;%f;%f;%f\n", angle, lv, lvs, rv, rvs);

    	serial_blue_write_string(buff);

    	counter++;
	}
}

void debug_ticks()
{
	//static unsigned int o = 0;
	static float o = 0;

	//unsigned int co = opto_get_tick_count();
	float co = tachometer_get_velocity_mps(BACK_RIGHT);
	unsigned int pwm = engine_get_pulse_width_pm(RIGHT);

	if (co != o)
	{
		char buff[64];

		o = co;

		//sprintf(buff, "Opto: %d\n", o);

		sprintf(buff, "%f\t%d\n", o, pwm);

		serial_blue_write_string(buff);
	}
}
