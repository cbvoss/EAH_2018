/*
 * tobys_controller_square.c
 *
 *  Created on: Mar 8, 2017
 *      Author: Tobias Hupel
 */
#include "enums.h"
#include "servo.h"
#include "Ir_Board.h"
#include "drive.h"
#include "drive_curve.h"
#include "distance_alarm.h"
#include "tachometer.h"

//#define FAST_JUMP
#define SLOW_JUMP

#if defined FAST_JUMP

#define BEFORE_JUMP_VELOCITY 1.8f               // 1.8f

#define FIRST_CURVE_OUTER_WHEEL_VELOCITY 1.8f   // 2.0f
#define SECOND_CURVE_OUTER_WHEEL_VELOCITY 1.8f  // 2.0f

#define FIRST_CURVE_BREAK_INNER_WHEEL 1         // 1
#define SECOND_CURVE_BREAK_INNER_WHEEL 1        // 1

#define SECOND_CURVE_MEMORY_MANIPULATION_ABS 3  // 3

#define SECOND_CURVE_FIXED_SERVO_DISTANCE 0.1f  // 0.1f
#define FINISHED_DISTANCE 0.5f                  // 0.5f

#elif defined SLOW_JUMP

#define JUMP_LINE_DETECT_PATTERN     0x3C    // 00111100

#define BEFORE_JUMP_VELOCITY 1.0f               // 1.8f

#define FIRST_CURVE_OUTER_WHEEL_VELOCITY 1.5f   // 1.8f
#define SECOND_CURVE_OUTER_WHEEL_VELOCITY 1.5f  // 1.8f

#define FIRST_CURVE_BREAK_INNER_WHEEL 1         // 1
#define SECOND_CURVE_BREAK_INNER_WHEEL 1        // 1

#define FIRST_JUMP_CURVE_SERVO_ANGLE 15
#define SECOND_JUMP_CURVE_SERVO_ANGLE 15

#define SECOND_CURVE_MEMORY_MANIPULATION_ABS 3  // 3

#define SECOND_CURVE_FIXED_SERVO_DISTANCE 0.1f  // 0.1f
#define FINISHED_DISTANCE 0.1f                  // 0.5f

#endif

enum ControllerJumpStates
{
    JUMP_EVENT_START, START_VELOCITY_REACHED, FIRST_JUMP_CURVE, SECOND_JUMP_CURVE, FINISHED_DISTANCE_DRIVEN
};

enum ControllerJumpStates g_jump_state = JUMP_EVENT_START;
enum Side g_jump_side = RIGHT;

struct DistanceAlarm g_finishedDistanceAlarm, g_fixedServoDistanceAlarm;

char g_jump_active = 0;
char g_jump_finished = 0;

void controller_jump_set_active(char active)
{
    g_jump_active = active;
}

void controller_jump_reset()
{
    g_jump_finished = 0;
    g_jump_active = 0;

    drive_curve_set_active (0);

    g_jump_state = JUMP_EVENT_START;
}

char controller_jump_has_finished()
{
    return g_jump_finished;
}

/**
 * @note
 *  Setzt die richtung in die der controller springen soll
 * @param side
 */
void controller_jump_set_jump_side(enum Side side)
{
    g_jump_side = side;
}

void controller_jump_state_machine()
{
    switch (g_jump_state)
    {
        case JUMP_EVENT_START:

            drive_curve_set_max_speed (BEFORE_JUMP_VELOCITY);
            drive_curve_set_active (1);

            if (!ir_get_pattern ())
            {
                g_jump_state = FIRST_JUMP_CURVE;

                drive_curve_set_active (0);

                servo_set_position_angle(g_jump_side == RIGHT ? -FIRST_JUMP_CURVE_SERVO_ANGLE : FIRST_JUMP_CURVE_SERVO_ANGLE);

                drive_accelerate_to_differential(BEFORE_JUMP_VELOCITY, 0);
            }

        break;

        case FIRST_JUMP_CURVE:

            if (ir_get_pattern () & JUMP_LINE_DETECT_PATTERN)
            {
                g_jump_state = SECOND_JUMP_CURVE;

                distance_alarm_new_specific (&g_finishedDistanceAlarm, FINISHED_DISTANCE, 1,
                                             g_jump_side == RIGHT ? BACK_RIGHT : BACK_LEFT);
//
//                distance_alarm_new_specific (&g_fixedServoDistanceAlarm, FINISHED_DISTANCE, 1,
//                                             g_jump_side == RIGHT ? BACK_RIGHT : BACK_LEFT);
//
//                drive_accelerate_to_advanced (g_jump_side == RIGHT ? LEFT : RIGHT, 0, SECOND_CURVE_BREAK_INNER_WHEEL,
//                                              0);
//                drive_accelerate_to_advanced (g_jump_side == RIGHT ? RIGHT : LEFT, SECOND_CURVE_OUTER_WHEEL_VELOCITY, 0,
//                                              0);

                /*drive_curve_manipulate_memory (
                        g_jump_side == RIGHT ? SECOND_CURVE_MEMORY_MANIPULATION_ABS :
                                               -SECOND_CURVE_MEMORY_MANIPULATION_ABS);*/
//                drive_curve_manipulate_memory(0);
            }

        break;

        case SECOND_JUMP_CURVE:

        	servo_set_position_angle(g_jump_side == LEFT ? -SECOND_JUMP_CURVE_SERVO_ANGLE : SECOND_JUMP_CURVE_SERVO_ANGLE);

            if (distance_alarm_has_distance_reached (&g_finishedDistanceAlarm))
                g_jump_state = FINISHED_DISTANCE_DRIVEN;

            // Manipulate Drive Curve
            drive_curve_manipulate_memory (0);

        break;

        case FINISHED_DISTANCE_DRIVEN:

            g_jump_finished = 1;

        break;
    }
}

void controller_jump_update(void)
{
    if (g_jump_active)
    {
        controller_jump_state_machine ();
    }
}
