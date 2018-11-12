/*
 * blue_remote_control.c
 *
 *  Created on: 09.01.2017
 *      Author: eric
 */

#include "serial_blue.h"
#include "servo.h"
#include "tachometer.h"
#include "drive.h"
#include "enums.h"
#include "global_clock.h"
#include "iodefine.h"
#include <stdio.h>

#define MaxCommandLen 31

/**
 * Moves the servo to the given position.
 * @param value
 * 	the position.
 */
void exec_move_servo(int value)
{
	char buffer[16];

	serial_blue_write_string("set servo to '");
	sprintf(buffer, "%d'\n", value);
	serial_blue_write_string(buffer);

	servo_set_position_pm(value);
}

/**
 * Moves the servo by the given amount.
 * @param value
 *  The position delta in steps.
 */
void exec_inc_servo(int value)
{
	value += servo_get_position_pm();
	servo_set_position_pm(value);
}

/**
 * Gets the distance moved so far. For both back wheels.
 */
void exec_get_dist()
{
	char buffer[16];

	serial_blue_write_string("BL");
	sprintf(buffer, "%f\n", tachometer_get_distance_meter(BACK_LEFT));
	serial_blue_write_string(buffer);

	serial_blue_write_string("BR");
	sprintf(buffer, "%f\n", tachometer_get_distance_meter(BACK_RIGHT));
	serial_blue_write_string(buffer);
}

/**
 * Gets the current servo position.
 */
void exec_get_servo()
{
	char buffer[16];

	serial_blue_write_string("servo ");
	sprintf(buffer, "%d\n", servo_get_position_pm());
	serial_blue_write_string(buffer);
}

/**
 * Moves the car by the given amount of centimeters.
 * @param value
 * 	The amount of way to move [cm]. Note: This is really rough since the break isn't used.
 */
void exec_move(int value)
{
	float distL, distR;
	float velocity;
	int absValue;

	velocity = (value < 0) ? -0.8 : 0.8;
	absValue = (value < 1) ? -value : value;

	distL = tachometer_get_distance_meter(BACK_LEFT) + ((float)absValue / 100);
	distR = tachometer_get_distance_meter(BACK_RIGHT) + ((float)absValue / 100);
	drive_accelerate_to(LEFT, velocity, 0);
	drive_accelerate_to(RIGHT, velocity, 0);

	while ((tachometer_get_distance_meter(BACK_LEFT) < distL) ||
			(tachometer_get_distance_meter(BACK_RIGHT) < distR))
	{ }

	drive_accelerate_to(LEFT, 0, 0);
	drive_accelerate_to(RIGHT, 0, 0);

	while ((tachometer_get_velocity_mps(BACK_LEFT) > 0.4) ||
			(tachometer_get_velocity_mps(BACK_RIGHT) > 0.4))
	{ }
}

/**
 * Moves one of the wheels by the given amount.
 * @param value
 *  The amount of way to move [cm]. Note: This is really rough since the break isn't used.
 * @param side
 *  The side of the wheel.
 */
void exec_move_side(int value, enum Side side)
{
	float dist;
	float velocity;
	int absValue;

	velocity = (value < 0) ? -0.8 : 0.8;
	absValue = (value < 1) ? -value : value;

	enum wheel_selector ws;
	ws = (side == LEFT ? BACK_LEFT : BACK_RIGHT);

	dist = tachometer_get_distance_meter(ws);
	dist += ((float)absValue / 100);
	drive_accelerate_to(side, velocity, 0);

	while (tachometer_get_distance_meter(ws) < dist)
 	{ }

	drive_accelerate_to(side, 0, 0);

	while (tachometer_get_velocity_mps(ws) > 0.4)
	{ }
}

/**
 * Gets the raw servo position (pwm). The servo calibration is based on this scaling.
 */
void exec_get_servo_raw()
{
	char buffer[16];
	int value;

	value = GPT0.GTCCRC;
	serial_blue_write_string("servo raw:");
	sprintf(buffer, "%d\n", value);
	serial_blue_write_string(buffer);
}

/**
 * Moves the servo relatively by the given number of ticks.
 * Warning: No collision checks. No limitations.
 * @param value
 */
void exec_inc_servo_raw(int value)
{
	GPT0.GTCCRC += value;
}

/**
 * Executes to execute a remote command.
 * @param cmd
 * 	The command. A usual c string.
 */
void execute_command(char* cmd)
{
	int value;
	long success;
	char buffer[16];

	//debug
	sprintf(buffer, "(%d)", global_clock_get_count());
	serial_blue_write_string(buffer);
	serial_blue_write_string("try exec cmd '");
	serial_blue_write_string(cmd);
	serial_blue_write_string("'...\n");

	success = sscanf(cmd, "servo%d", &value);
	if (success)
	{
		exec_move_servo(value);
		return;
	}

	success = sscanf(cmd, "servoinc%d", &value);
	if (success)
	{
		exec_inc_servo(value);
		return;
	}

	success = sscanf(cmd, "getservo%d", &value);
	if (success)
	{
		exec_get_servo();
		return;
	}

	success = sscanf(cmd, "getservoraw%d", &value);
	if (success)
	{
		exec_get_servo_raw();
		return;
	}

	success = sscanf(cmd, "servoincraw%d", &value);
	if (success)
	{
		exec_inc_servo_raw(value);
		return;
	}

	success = sscanf(cmd, "getdist%d", &value);
	if (success)
	{
		exec_get_dist();
		return;
	}

	success = sscanf(cmd, "movel%d", &value);
	if (success)
	{
		exec_move_side(value, LEFT);
		return;
	}

	success = sscanf(cmd, "mover%d", &value);
	if (success)
	{
		exec_move_side(value, RIGHT);
		return;
	}

	success = sscanf(cmd, "move%d", &value);
	if (success)
	{
		exec_move(value);
		return;
	}

	// in case we don't understand the command
	serial_blue_write_string("ignored command '");
	serial_blue_write_string(cmd);
	serial_blue_write_string("'\n");
}

/**
 * Trys to fetch a command from serial blue and executes it.
 */
void remote_control_update()
{
	static char command[MaxCommandLen + 1];	//+1 because of 0-character
	static char commandLength = 0;
	char currentChar;
	char commandComplete;

	commandComplete = 0;
	do
	{
		currentChar = serial_blue_pull_receive_buffer();

		if (currentChar == ';')
		{
			commandComplete = 1;
			break;
		}

		if (currentChar != 0)
		{
			command[commandLength] = currentChar;
			++commandLength;
		}
	} while ((currentChar != 0) && (commandLength < MaxCommandLen));

	if (commandComplete || (commandLength >= MaxCommandLen))
	{
		command[commandLength] = 0;
		execute_command(command);
		commandLength = 0;
	}
}
