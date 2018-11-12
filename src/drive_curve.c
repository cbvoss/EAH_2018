/*
 * drive_curve.c
 *
 *  Created on: 07.01.2017
 *      Author: franz_luebke
 *              Adrian Zentgraf
 *      Review:
 *      Florian Schädlich
 *      Adrian Zentgraf
 *      Franz Lübke
 *
 */
#include "config_car.h"
#include "drive_curve.h"
#include "Ir_Board.h"
#include "servo.h"
#include "global_clock.h"
#include "tachometer.h"
#include "distance_alarm.h"
#include "adc.h"
#include "drive.h"
#include "regulator.h"
#include "typedefine.h"
#include "iodefine.h"
#include "fuzzy.h"
#include "math_tools.h"
#include "serial_blue.h"
#include <math.h>
#include <stdio.h>

//#define USE_FUZZY
#define FUZZY_DISTANCE_DA_M 0.01f

#define ADC_OFFSET 128
#define ADC_MAX_ABS_VALUE 128

#define PI 3.141592f				// Without Weighting
#define SERVO_REGULATOR_V 3.0f		// 2.0f
#define SERVO_REGULATOR_I 0.0f		// 10.0f
#define SERVO_REGULATOR_D 0.0f
#define SERVO_REGULATOR_WINDUP_BORDER 90.0f	//500.0f

//#define SERVO_REGULATOR_LIVE_CONFIGURATION

#define SERVO_REGULATOR_DEVIATION_CUT_OFF 8

/**
 * Weight IR Board Deviation
 */
 /** START */

#define SERVO_REGULATOR_WEIGHT_IR_DEVIATION

#define SERVO_REGULATOR_LOWER_WEIGHT_BORDER 2
#define SERVO_REGULATOR_LOWER_WEIGHT (2.0f / SERVO_REGULATOR_V)

#define SERVO_REGULATOR_HIGHER_WEIGHT_BORDER 6
#define SERVO_REGULATOR_HIGHER_WEIGHT (3.75f / SERVO_REGULATOR_V)

/** END */

#define CURVE_MEMORY_BUFFER_SIZE 64
#define DRIVE_CURVE_USE_BREAK 0

#define MAX_SERVO_ANGLE_ABS 55.0	//maximum servo angle while following the line

float g_curve_max_speed = 1.5f;
char g_drive_curve_active = 0;
struct DistanceAlarm g_curve_distance, g_fuzzy_da;
struct Regulator g_curve_servo_regulator;
struct Regulator g_wheel_regulator;
struct Memory g_curve_servo_position_memory;
struct timer g_curve_timer;

int g_max_servo_angle = 45;

#ifdef DEBUG_DRIVE_CURVE

float g_max_set_value = REGULATOR_V*ADC_MAX_ABS_VALUE+2.0f*REGULATOR_WINDUP_BORDER;
float SERVO_REGULATOR_WINDUP_BORDER =0;

void drive_curve_set_regulator(float V, float I, float D)
{

    regulator_new(&g_wheel_regulator,V, I, D, 0.001f);
    g_max_set_value = V*ADC_MAX_ABS_VALUE+2.0f*REGULATOR_WINDUP_BORDER;
}

void drive_curve_set_servo_regulator(float V, float I, float D)
{
    regulator_new(&g_curve_servo_regulator,V, I, D, 0.001f);
}

#endif

float drive_curve_get_I()
{
    return g_curve_servo_regulator.deviation_sum;
}

/**
 * clears the memory struct with all 0
 * @param pointer to Memory struct
 */
void drive_curve_reset_memory(struct Memory *mem)
{
    int i;
    for (i = 0; i < CURVE_MEMORY_BUFFER_SIZE; i++)
    {
        mem->buffer[i] = 0;
    }
    mem->sum = 0;
    mem->w_ptr = 0;
}

/**
 * activates xor deactivates the whole drive curve module.
 * @param active
 */
void drive_curve_set_active(char active)
{
    g_drive_curve_active = active ? 1 : 0;
}

/**
 * put int values in a buffer to calculate the moving average of these values
 * @param pointer to Memory struct ->*mem
 * @param value to put -> pm_angle
 * @return the sum of the buffer elements
 */
int drive_curve_memory(struct Memory *mem, int pm_angle)
{
    pm_angle = (pm_angle > 1000) ? 1000 : pm_angle;
    pm_angle = (pm_angle < -1000) ? -1000 : pm_angle;
    mem->buffer[mem->w_ptr++] = pm_angle;
    mem->w_ptr &= CURVE_MEMORY_BUFFER_SIZE - 1;
    mem->sum += (pm_angle - (mem->buffer[mem->w_ptr]));

    return mem->sum;
}

/**
 * get the sum of the memory buffer
 * @param pointer to a memory struct -> *mem
 * @return sum of the memory buffer values
 */
int drive_curve_get_memory(struct Memory *mem)
{
    return mem->sum;
}

/**
 * updates servo deviation in dependents of the given ir pattern in the memmory
 * @param ir_pattern
 * @return void
 */
void drive_curve_update_memory(unsigned char ir_pattern)
{
    drive_curve_memory (&g_curve_servo_position_memory, drive_curve_ir_deviation (ir_pattern));
}

char drive_curve_good_position()
{
    const int Epsilon = 230;
    int mem = drive_curve_get_memory (&g_curve_servo_position_memory);
    return ((mem > (-1 * Epsilon)) && (mem < Epsilon));
}

#ifdef DEBUG
int drive_curve_get_memory_debug()
{
    return g_curve_servo_position_memory.sum;
}
#endif

/**
 * set all drive curve values to initial values
 */
void drive_curve_reset()
{
    drive_curve_reset_memory (&g_curve_servo_position_memory);
    regulator_new (&g_curve_servo_regulator, SERVO_REGULATOR_V, SERVO_REGULATOR_I, SERVO_REGULATOR_D, 0.001f);
    regulator_set_windup (&g_curve_servo_regulator, SERVO_REGULATOR_WINDUP_BORDER);
}

/**
 * calculates the regulator output
 * @return regulator output
 */
float drive_curve_servo_regulator()
{
    float servo_derivation = (float) drive_curve_ir_deviation (ir_get_pattern ());

#ifdef SERVO_REGULATOR_DEVIATION_CUT_OFF

    if (fabsf(servo_derivation) > SERVO_REGULATOR_DEVIATION_CUT_OFF)
    	servo_derivation = sign_f(servo_derivation) * SERVO_REGULATOR_DEVIATION_CUT_OFF;

#endif

#ifdef SERVO_REGULATOR_WEIGHT_IR_DEVIATION

    if (fabsf(servo_derivation) <= SERVO_REGULATOR_LOWER_WEIGHT_BORDER)
    {
    	servo_derivation *= SERVO_REGULATOR_LOWER_WEIGHT;
    }
    else if (fabsf(servo_derivation) >= SERVO_REGULATOR_HIGHER_WEIGHT_BORDER)
    {
    	servo_derivation *= SERVO_REGULATOR_HIGHER_WEIGHT;
    }

#endif

    return regulator_calculate_value (&g_curve_servo_regulator, servo_derivation);
}

/**
 * Set the maximal speed of drive curve.
 * @param speed in m/s
 */
void drive_curve_set_max_speed(float speed)
{
    g_curve_max_speed = speed;
}

/**
 * Initialize the module
 */
void drive_curve_initialize()
{
    distance_alarm_new_mean (&g_curve_distance, 0.001f, 1);
    distance_alarm_new_mean(&g_fuzzy_da, FUZZY_DISTANCE_DA_M, 1);
    regulator_new (&g_curve_servo_regulator, SERVO_REGULATOR_V, SERVO_REGULATOR_I, SERVO_REGULATOR_D, 0.001f);
    regulator_set_windup (&g_curve_servo_regulator, SERVO_REGULATOR_WINDUP_BORDER);
    drive_curve_reset_memory (&g_curve_servo_position_memory);
    PORTD.DDR.BIT.B0 = 1;
    PORTD.DR.BIT.B0 = 1;
    global_clock_set_timer (&g_curve_timer, 3);
}

/**
 * The logical actuator of the back wheel position regulation.
 * It slows down a wheel depending on the given regulator output
 */
void drive_curve_controller_switching_wheel(){
	//enum Side side;
	int line_error = servo_get_position_pm();
	if (line_error < 0) line_error = -line_error;
	float line_error_norm = (float) (line_error / 1000);

	drive_accelerate_to_differential_advanced(g_curve_max_speed, 0, 0.2f);

//	for (side = RIGHT; side <= LEFT; side ++){
//		if (line_error < 200)
//			drive_accelerate_to(side, g_curve_max_speed, 0);
//		else
//			drive_accelerate_to(side, (1.2 - (line_error_norm/2)) * g_curve_max_speed, 0);
//	}
}

/**
 * set a Memory struct to a given value
 * @param value to be set in each buffer slot
 */
void drive_curve_manipulate_memory(int mem)
{
    g_curve_servo_position_memory.sum = 0;
    for (int i = 0; i < 64; i++)
    {
        g_curve_servo_position_memory.buffer[i] = mem;
        g_curve_servo_position_memory.sum += mem;
    }
    g_curve_servo_position_memory.w_ptr = 0;
}

/**
 * function to update the servo position
 */
void drive_curve_update_fixed()
{
#ifndef USE_FUZZY
	if(g_drive_curve_active){
		float servo_angle = drive_curve_servo_regulator();

#ifdef MAX_SERVO_ANGLE_ABS

		//limit the servo angle to the maximum angle
		if(fabsf(servo_angle) > MAX_SERVO_ANGLE_ABS)
			servo_angle = sign_f(servo_angle) * MAX_SERVO_ANGLE_ABS;

#endif

		servo_set_position_angle(servo_angle);
	}
#endif
}

/**
 * main function to use drive curve
 */
void drive_curve_update()
{
#ifdef USE_FUZZY
//	if (distance_alarm_has_distance_reached(&g_fuzzy_da))
//	{
//		distance_alarm_reset(&g_fuzzy_da);

		if (g_drive_curve_active)
		{
			float servoAngle_deg = fuzzy_calculate_servo_angle(ir_get_pattern(), servo_get_position_angle_deg(), tachometer_get_arithemtic_mean_velocity_mps());

			servo_set_position_angle(servoAngle_deg);
		}
//	}
#endif

	if (distance_alarm_has_distance_reached (&g_curve_distance))
    {
        distance_alarm_reset (&g_curve_distance);

        if (g_drive_curve_active)
        {
        	//distance-depending code
        	drive_curve_controller_switching_wheel();
#ifdef DEBUG_DRIVE_CURVE
            if(global_clock_timer(&g_curve_timer))
            {
                global_clock_reset_timer(&g_curve_timer);

                char str[100];
                snprintf(str,100,"#mem %d \r\n#i_sum %.3f\r\n#servo %d\r\n ", drive_curve_get_memory(&g_curve_servo_position_memory), g_curve_servo_smooth_regulator.deviation_sum,servo_get_position_pm());
                serial_blue_write_string(str);
            }
#endif
        }

        drive_curve_update_memory (ir_get_pattern ());
    }

#ifdef SERVO_REGULATOR_LIVE_CONFIGURATION
	char received = serial_blue_pull_receive_buffer();

	if (received == '+')
	{
		g_curve_servo_regulator.v += 0.1f;
	}
	else if (received == '-')
	{
		g_curve_servo_regulator.v -= 0.1f;
	}
	else if (received == '6')
	{
		g_curve_servo_regulator.ti_inverse += 0.1f;
	}
	else if (received == '9')
	{
		g_curve_servo_regulator.ti_inverse -= 0.1f;
	}
	else if (received == 'p')
	{
		char buff[64];
		sprintf(buff, "V: %f - I: %f\n", g_curve_servo_regulator.v, g_curve_servo_regulator.ti_inverse);
		serial_blue_write_string(buff);
	}
#endif
}
/**
 * calculates the servo deviation in dependents of the given ir pattern
 * @param ir_pattern
 * @return deviation
 */
int drive_curve_ir_deviation(unsigned char ir_pattern)
{
    switch (ir_pattern)
    {
        case 0x18:  //00011000
            return -0;
        case 0x1c:  //00011100
            return -1;
        case 0x8:	//00001000
        	return -1;
        case 0x0c:  //00001100
            return -2;
        case 0x0e:  //00001110
            return -3;
        case 0x4:	//00000100
        	return -3;
        case 0x06:  //00000110
            return -4;
        case 0x07:  //00000111
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 9 : -5;
        case 0x2:	//00000010
        	return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 9 : -5;
        case 0x03:  //00000011
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 9 : -6;
        case 0x01:  //00000001
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 8 : -7;
        case 0x83: //10000011
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 7 : -6;
        case 0x00:  //00000000
            return drive_curve_get_memory (&g_curve_servo_position_memory) / CURVE_MEMORY_BUFFER_SIZE;
        case 0x81:  //10000001
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 7 : -7;
        case 0xc1: //11000001
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 6 : -7;
        case 0x80:  //10000000
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 7 : -8;
        case 0xc0:  //11000000
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 6 : -9;
        case 0xe0:  //11100000
            return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 5 : -9;
        case 0x40:	//01000000
        	return (drive_curve_get_memory (&g_curve_servo_position_memory) > 0) ? 5 : -9;
        case 0x60:  //01100000
            return 4;
        case 0x70:  //01110000
            return 3;
        case 0x20:	//00100000
        	return 3;
        case 0x30:  //00110000
            return 2;
        case 0x38:  //00111000
            return 1;
        case 0x10:	//00010000
        	return 1;
        default:
            return drive_curve_get_memory (&g_curve_servo_position_memory) / CURVE_MEMORY_BUFFER_SIZE;
    }
}
