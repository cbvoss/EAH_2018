/*
 * controller_square.c
 *
 *  Created on: 05.01.2017
 *      Author: Tobias Hupel
 *      		Eric Elsner
 * 
 *  Edited on:  2018-02-21
 *      Author: Andreas Chwojka
 */

#include "config_car.h"
#include "distance_alarm.h"
#include "drive.h"
#include "drive_curve.h"
#include "enums.h"
#include "fuzzy_sniffer.h"
#include "Ir_Board.h"
#include "serial_blue.h"
#include "servo.h"
#include "tachometer.h"
#include "serial_blue.h"
#include "regulator.h"
#include <math.h>

#define CONTROLLER_SQUARE_DEBUG
//#define CONTROLLER_SQUARE_TRACE


#define LINE_DETECT_PATTERN_LEFT	0x1C	// 00011100
#define LINE_DETECT_PATTERN_RIGHT	0x38	// 00111000
#define TURN_SIDE_RIGHT_PATTERN 	0x03 	// 00000011
#define TURN_SIDE_LEFT_PATTERN  	0xC0 	// 11000000


//#define IGNORE_DIRECTION_DISTANCE 0.01f	// ignore patterns if square just started
#define MINIMUM_STEERING_DISTANCE 0.3f	// minimum distance to end controller


#define TRUE 0x01
#define FALSE 0x00

#define MEMORY_MANIPULATION_ABS 	3               // 3
//#define SERVO_PULSE_WIDTH_ABS 		1000            // 1000

// Regulate Angle
#define SQUARE_REGULATE_ANGLE
#define SQUARE_REGULATE_ANGLE_DA_DISTANCE 0.001f
#define SQUARE_REGULATE_ANGLE_MIN_ANGLE_ABS 15.0f

#define SQUARE_SERVO_REGULATOR_V 3.0f
#define SQUARE_SERVO_REGULATOR_I 10.0f
#define SQUARE_SERVO_REGULATOR_D 0.0f
#define SQUARE_SERVO_REGULATOR_WINDUP_BORDER 500.0f //500.0f

// TEST: Dynamic Angle
#define DYNAMIC_ANGLE
#define D_A_FIST			15.0f					// first angle
#define	D_A_FACTOR_1		1.4f					// increase factor
#define D_A_FACTOR          0.98f                   // decrease factor, new angle = old angle * D_A_FACTOR
#define D_A_DISTANCE        0.03f                   // iteration distance
#define D_A_BOARDER         25.0f                   // 30 degrees lowest angle
#define D_A_VELOCTY_FACTOR   1.03f
#define D_A_VELOCTY_MAX	     1.0f

// Fallback values if not defined in Config_Car

#ifndef SQUARE_ANGLE_LEFT
    #define SQUARE_ANGLE_LEFT 			+60.0f  //degrees
#endif
#ifndef SQUARE_ANGLE_RIGHT
    #define SQUARE_ANGLE_RIGHT 	 		-60.0f  //degrees
#endif
#ifndef SQUARE_VELOCITY
	#define SQUARE_VELOCITY			  	  1.0f  //m/s
#endif



enum ControllerSquareStates
{
    S0_Idle,
	S1_Detect_Direction,
	S2a_Left_Direction, S2b_Right_Direction,
	S3_Wait_distance,
	S4_Steering, S5_dynamic_Angle,
	S6_Completed,
	S7_Reset,
	S8_Regulate_Angle
};

struct ControllerSquare
{
    // Controller States
    char active, finished, reset;
    enum ControllerSquareStates last_state, state, new_state;

    // Speed Setting
    float velocity;

    // Curve information
    float turn_angle;

    // Wheel for distance alarms
    enum wheel_selector wheel;

    // Target Pattern to reactivate drive control
    char pattern;

    // Max Angle reached
    char max_reached;

    // Distance Alarms
    struct DistanceAlarm distanceAlarm; //fixedServoDistanceAlarm, minimumSteeringDistanceAlarm;

	#ifdef DYNAMIC_ANGLE
		struct DistanceAlarm dynamicAngleDistanceAlarm;
	#endif

	#ifdef SQUARE_REGULATE_ANGLE
		struct Regulator servoRegulator;
		struct DistanceAlarm regulateAngleDA;
	#endif

	#ifdef CONTROLLER_SQUARE_TRACE

		unsigned char trace_cnt;
		struct DistanceAlarm traceDistanceAlarm;
		char trace [100];

	#endif

} g_controller_square;

void controller_square_initialize()
{
     g_controller_square.new_state = S7_Reset;
     g_controller_square.state = S7_Reset;
}

void controller_square_set_active(char active)
{
    g_controller_square.active = active;
	if (!active) g_controller_square.reset = 1;	// Reset Controller
}

char controller_square_is_active()
{
    return g_controller_square.active;
}

char controller_square_has_finished()
{
    return g_controller_square.finished;
}

//int get_max_servo_pulse_width(enum Side side)
//{
//    return side == RIGHT ? -SERVO_PULSE_WIDTH_ABS : SERVO_PULSE_WIDTH_ABS;
//}

// Controller Update State Machine
void controller_square_update()
{
	char ir_pattern;

	switch (g_controller_square.state)
	{
		// Idle, do nothing -> S0 / S1
		case S0_Idle:

			if (g_controller_square.active==1) g_controller_square.new_state = S1_Detect_Direction;
			break;


		// Detect Left/Right direction -> S1 / S2a / S2b
		case S1_Detect_Direction:

			// other state  -> S1
			if (g_controller_square.last_state != S1_Detect_Direction)
			{
				drive_curve_set_max_speed (g_controller_square.velocity);	// Set Max Speed using Drive Curve
				drive_curve_set_active (1);

				#ifdef IGNORE_DIRECTION_DISTANCE
					distance_alarm_new_specific (&(g_controller_square.distanceAlarm), IGNORE_DIRECTION_DISTANCE, 1, g_controller_square.wheel);
				#endif

				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S1: Start...\n");
				#endif
			}

			#ifdef IGNORE_DIRECTION_DISTANCE
				if (distance_alarm_has_distance_reached (&(g_controller_square.distanceAlarm)) )
					{
					// Wait for Pattern (NEW)
					// if (ted_square_detected() == TRUE)
					// {
					// 	if (ted_get_square_side() == LEFT) g_controller_square.new_state = S2a_Left_Direction;
					// 	else g_controller_square.new_state = S2b_Right_Direction;

					// Wait for Pattern (OLD)
					ir_pattern = ir_get_pattern();
					if ((ir_pattern & TURN_SIDE_LEFT_PATTERN) == TURN_SIDE_LEFT_PATTERN)
					{
						g_controller_square.new_state = S2a_Left_Direction;
					}
					else if ((ir_pattern & TURN_SIDE_RIGHT_PATTERN) == TURN_SIDE_RIGHT_PATTERN)
					{
						g_controller_square.new_state = S2b_Right_Direction;
					}
				}
			#else
				// Wait for Pattern (OLD)
				ir_pattern = ir_get_pattern();
				if ((ir_pattern & TURN_SIDE_LEFT_PATTERN) == TURN_SIDE_LEFT_PATTERN)
				{
					g_controller_square.new_state = S2a_Left_Direction;
				}
				else if ((ir_pattern & TURN_SIDE_RIGHT_PATTERN) == TURN_SIDE_RIGHT_PATTERN)
				{
					g_controller_square.new_state = S2b_Right_Direction;
				}
			#endif
			break;


		// Direction specific settings -> S3
		case S2a_Left_Direction:

				g_controller_square.wheel = BACK_RIGHT;
				g_controller_square.pattern = LINE_DETECT_PATTERN_LEFT;
				g_controller_square.new_state = S3_Wait_distance;

				#ifdef DYNAMIC_ANGLE
					g_controller_square.turn_angle = D_A_FIST;
				#else
					g_controller_square.turn_angle = SQUARE_ANGLE_LEFT;
				#endif

				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S2a: Left Side\n");
				#endif
			break;

		case S2b_Right_Direction:

				g_controller_square.wheel = BACK_LEFT;
				g_controller_square.pattern = LINE_DETECT_PATTERN_RIGHT;
				g_controller_square.new_state = S3_Wait_distance;

				#ifdef DYNAMIC_ANGLE
					g_controller_square.turn_angle = -D_A_FIST;
				#else
					g_controller_square.turn_angle = SQUARE_ANGLE_RIGHT;
				#endif

				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S2b: Right Side\n");
				#endif
			break;


		// Wait for distance (+ servo reaction distance!) -> S3 / S4
		case S3_Wait_distance:

			// If there's no distance to wait, jump to next state!
			#ifdef SQUARE_DISTANCE_TO_STEER
				// Init Tachometer
				if ((g_controller_square.last_state == S2a_Left_Direction) ||
					(g_controller_square.last_state == S2b_Right_Direction))
				{
					distance_alarm_new_specific (&(g_controller_square.distanceAlarm), SQUARE_DISTANCE_TO_STEER, 1, g_controller_square.wheel);
				}

				// Distance driven
				if (distance_alarm_has_distance_reached (&(g_controller_square.distanceAlarm)) )
					g_controller_square.new_state = S4_Steering;
			#else
				g_controller_square.new_state = S4_Steering;
			#endif

			break;


		// Steering -> S4 / S5 / S6
		case S4_Steering:

			// S3 -> S4
			if (g_controller_square.last_state == S3_Wait_distance)
			{
				drive_curve_set_active (0);		// Disable Drive Curve

				drive_accelerate_to_differential(g_controller_square.velocity, 0);

				// Init Tachometer: min Steering Distance
				distance_alarm_new_specific (&(g_controller_square.distanceAlarm), MINIMUM_STEERING_DISTANCE, 1, g_controller_square.wheel);

				// Reactivate Drive Curve
				//drive_curve_set_active (1);

				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S4: Steering...\n");
				#endif
			}

			// Dynamic Angle
			#ifdef DYNAMIC_ANGLE
				if (g_controller_square.last_state != S4_Steering)
				{
					distance_alarm_new_specific (&(g_controller_square.dynamicAngleDistanceAlarm), D_A_DISTANCE, 1, g_controller_square.wheel);
				}
				else {
					if (distance_alarm_has_distance_reached (&(g_controller_square.dynamicAngleDistanceAlarm)))
						g_controller_square.new_state = S5_dynamic_Angle;
				}
			#endif

			// Set Servo
			servo_set_position_angle (g_controller_square.turn_angle);

			// Manipulate Drive Curve
			if (g_controller_square.turn_angle < 0)
                    drive_curve_manipulate_memory (-MEMORY_MANIPULATION_ABS);
                else
                    drive_curve_manipulate_memory (MEMORY_MANIPULATION_ABS);

			// Get IR Pattern
			ir_pattern = ir_get_pattern();

			// Complete Condition <- min Steering Distance & Pattern
			if (distance_alarm_has_distance_reached (&(g_controller_square.distanceAlarm)))
					{
						//serial_blue_write_string("S4: Distance OK\n");

						if ((ir_pattern & g_controller_square.pattern) > 0)
						{
							//serial_blue_write_string("S4: Pattern OK\n");
							#if defined(SQUARE_REGULATE_ANGLE)
								g_controller_square.new_state = S8_Regulate_Angle;
							#else
								g_controller_square.new_state = S6_Completed;
							#endif
						}
					}

			// Trace IR Patterns per cm
			#ifdef CONTROLLER_SQUARE_TRACE

				if (g_controller_square.trace_cnt == 0)
					distance_alarm_new_specific (&(g_controller_square.traceDistanceAlarm), 0.01f, 1, g_controller_square.wheel);


				if (distance_alarm_has_distance_reached (&(g_controller_square.traceDistanceAlarm))
						&& (g_controller_square.trace_cnt < 100) )
				{
					g_controller_square.trace[g_controller_square.trace_cnt++] = ir_pattern;
					distance_alarm_new_specific (&(g_controller_square.traceDistanceAlarm), 0.01f, 1, g_controller_square.wheel);
				}
			#endif

			break;


		// Dynamic angle adjustment -> S4
		case S5_dynamic_Angle:
			// P Regulator


			if (g_controller_square.max_reached == 0)
			{
				g_controller_square.turn_angle *= D_A_FACTOR_1;

				if ( (g_controller_square.turn_angle < 0.0f) && (g_controller_square.turn_angle > SQUARE_ANGLE_RIGHT) )
				{
						g_controller_square.turn_angle = SQUARE_ANGLE_RIGHT;
						g_controller_square.max_reached = 1;
				}
				if ( (g_controller_square.turn_angle > 0.0f) && (g_controller_square.turn_angle < SQUARE_ANGLE_LEFT) )
				{
						g_controller_square.turn_angle = SQUARE_ANGLE_LEFT;
						g_controller_square.max_reached = 1;
				}
			}
			else
			{
				g_controller_square.turn_angle *= D_A_FACTOR;

				// Boarders
				if ( (g_controller_square.turn_angle < 0.0f) && (g_controller_square.turn_angle > -D_A_BOARDER) )
										g_controller_square.turn_angle = -D_A_BOARDER;
				if ( (g_controller_square.turn_angle > 0.0f) && (g_controller_square.turn_angle < D_A_BOARDER) )
										g_controller_square.turn_angle = D_A_BOARDER;

				if (g_controller_square.velocity < D_A_VELOCTY_MAX)
				{
					g_controller_square.velocity *= D_A_VELOCTY_FACTOR;
					drive_curve_set_max_speed (g_controller_square.velocity);
				}
			}

			g_controller_square.new_state = S4_Steering;

			#ifdef CONTROLLER_SQUARE_DEBUG
				serial_blue_write_string("S5: Dynamic\n");
			#endif

			break;


		// Regulate angle to normal driving angles -> S6
		case S8_Regulate_Angle:

		#ifdef SQUARE_REGULATE_ANGLE

			if (g_controller_square.last_state == S4_Steering)
			{
				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S8: Regulate Angle\n");
				#endif
			}

			float servoAngle_deg = servo_get_position_angle_deg();

			if (fabsf(servoAngle_deg) > SQUARE_REGULATE_ANGLE_MIN_ANGLE_ABS)
			{
				if (distance_alarm_has_distance_reached(&(g_controller_square.regulateAngleDA)))
				{
					distance_alarm_reset(&(g_controller_square.regulateAngleDA));

					float regValue = regulator_calculate_value(&(g_controller_square.servoRegulator),
							drive_curve_ir_deviation(ir_get_pattern()));

					servo_set_position_angle(regValue);
				}
			}
			else
			{
				g_controller_square.new_state = S6_Completed;
			}

			break;
		#else
			g_controller_square.new_state = S6_Completed;

			break;
		#endif
		// Square completely driven -> S6
		case S6_Completed:
			if (g_controller_square.last_state != S6_Completed)
			{
				g_controller_square.finished = 1;

				#ifdef CONTROLLER_SQUARE_DEBUG
					serial_blue_write_string("S6: Completed\n");
				#endif

				#ifdef CONTROLLER_SQUARE_TRACE
					for (char cnt=0; cnt < g_controller_square.trace_cnt; cnt++)
						serial_blue_write_string(&g_controller_square.trace[cnt]);
				#endif
			}
			break;


		// Reset -> S0
		case S7_Reset:
			g_controller_square.active = 0;
			g_controller_square.finished = 0;
			g_controller_square.reset = 0;
			g_controller_square.max_reached = 0;

			g_controller_square.velocity = SQUARE_VELOCITY;
			g_controller_square.turn_angle = 0.0f;
			g_controller_square.wheel = BACK_LEFT;

			g_controller_square.pattern = 0;

			g_controller_square.new_state = S0_Idle;

			#ifdef SQUARE_REGULATE_ANGLE
				regulator_new(&(g_controller_square.servoRegulator), SQUARE_SERVO_REGULATOR_V, SQUARE_SERVO_REGULATOR_I,
						SQUARE_SERVO_REGULATOR_D, SQUARE_REGULATE_ANGLE_DA_DISTANCE);
				regulator_set_windup(&(g_controller_square.servoRegulator), SQUARE_SERVO_REGULATOR_WINDUP_BORDER);

				distance_alarm_new_mean(&(g_controller_square.regulateAngleDA), SQUARE_REGULATE_ANGLE_DA_DISTANCE, 1);
			#endif

			#ifdef CONTROLLER_SQUARE_DEBUG
				serial_blue_write_string("S7: Reset\n");
			#endif

			#ifdef CONTROLLER_SQUARE_TRACE

				g_controller_square.trace_cnt = 0;

				for (char cnt = 0; cnt<100; cnt++)
					g_controller_square.trace [cnt];

			#endif

			break;


		// Unknown States -> S0
		default:

			g_controller_square.new_state = S0_Idle;
			break;
	}

	// Change current/last State
	g_controller_square.last_state = g_controller_square.state;
	g_controller_square.state = g_controller_square.new_state;

	// Reset
	if (g_controller_square.reset == 1)
	{
		g_controller_square.reset = 0;
		g_controller_square.state = S7_Reset;
		g_controller_square.new_state = S7_Reset;
	}

}


