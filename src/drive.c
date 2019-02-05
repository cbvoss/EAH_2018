/*
 * drive.c
 *
 *  Created on: 02.12.2016
 *      Author: Tobias Hupel
 *      		Eric Elsner
 *
 *      Reviewed on 05.01.2017 by:
 *      		Tobias Hupel
 *      		Eric Elsner
 *      		Adrian Zentgraf
 *
 *      Extension Reviewed on 26.01.2017 by
 *      		Eric Elsner
 *      		Tobias Hupel
 */

#include "drive.h"
#include "tachometer.h"
#include "differential.h"
#include "config_car.h"
#include "servo.h"
#include <math.h>
#include "math_tools.h"
#include "opto_interruptor.h"

#define FIXED_UPDATE_TIME_STEP_S 0.001f					// Fixed Update Time Interval
#define VELOCITY_REGULATOR_V 2000.0f					// Amplification of velocity regulator
#define VELOCITY_REGULATOR_TI_INVERSE 0.35f				// Ti inverse of velocity regulator
#define VELOCITY_REGULATOR_TD 0.0f						// Td of velocity regulator
#define VELOCITY_REGULATOR_WINDUP 1000//5

#define VELOCITY_REGULATOR_OUTPUT_SCALE_FACTOR 1.0f	// Scales the result

#define VELOCITY_TO_ZERO_TOLERANCE_MPS 0.3f				// velocity about 0.0mps is impossible to detect
#define DEFAULT_BREAK_THRESHOLD_MPS -0.1f				// default delta defining the velocity difference threshold for breaking

regulation_targets_t g_regulation_targets;

// Differential Part
char g_differential_active;
float g_differential_target_velocity_mps;

void drive_initialize()
{
    enum Side side;

    for (side = RIGHT; side <= LEFT; side++)
    {
        g_regulation_targets.engines[side].abs_target_velocity_mps = 0;
        g_regulation_targets.engines[side].break_threshold_mps = DEFAULT_BREAK_THRESHOLD_MPS;
        g_regulation_targets.engines[side].use_regulator_break = 0;
        g_regulation_targets.engines[side].target_direction_mode = FORWARD_FREERUN;
        g_regulation_targets.engines[side].target_direction_mode_changed = 0;
        regulator_new (&g_regulation_targets.engines[side].regulator, VELOCITY_REGULATOR_V,
                       VELOCITY_REGULATOR_TI_INVERSE,
                       VELOCITY_REGULATOR_TD,
                       FIXED_UPDATE_TIME_STEP_S);
        regulator_set_windup(&g_regulation_targets.engines[side].regulator, VELOCITY_REGULATOR_WINDUP);
        g_regulation_targets.engines[side].active_break = 0;
        g_regulation_targets.engines[side].active_break_pw = 0;
    }

    g_differential_active = 0;
    g_differential_target_velocity_mps = 0;
}

/**
 * Returns whether differential is active or not.
 *
 * @return
 * 		"1" if differential is active, otherwise "0"
 */
char drive_is_differential_active()
{
	return g_differential_active;
}

/**
 * Returns the target velocity of the differential calculation.
 *
 * @return
 * 		the target velocity in meter per second.
 */
float drive_get_differential_target_velocity_mps()
{
	return g_differential_target_velocity_mps;
}

/**
 *	Returns the target velocity of the specified wheel.
 *
 * @param side
 * 		the side of the wheel
 * @return
 * 		the target velocity of the wheel in meter per second
 */
float drive_get_wheel_target_velocity_mps(enum Side side)
{
	return g_regulation_targets.engines[side].abs_target_velocity_mps;
}

/**
 * Accelerates to the given velocity using the differential functionality.
 *
 * @param velocity_mps
 * 		the target velocity in meter per second
 * @param use_break
 * 		should the regulator use the engine break
 */
void drive_accelerate_to_differential(float velocity_mps, char use_break)
{
	drive_accelerate_to_differential_advanced(velocity_mps, use_break, DEFAULT_BREAK_THRESHOLD_MPS);
}

/**
 * Accelerates to the given velocity using the differential functionality.
 *
 * @param velocity_mps
 * 		the target velocity in meter per second
 * @param use_break
 * 		should the regulator use the engine break
 * @param break_threshold
 * 		the threshold when to use the engine break
 */
void drive_accelerate_to_differential_advanced(float velocity_mps, char use_break, float break_threshold)
{
	enum OperationMode target_mode = FORWARD_FREERUN; //velocity_mps < 0 ? BACKWARD_FREERUN : FORWARD_FREERUN;

	//	if (g_regulation_targets.engines[side].target_direction_mode != target_mode)
	//		g_regulation_targets.engines[side].target_direction_mode_changed = 1;
	for (enum Side side = RIGHT; side <= LEFT; side++)
	{
	    g_regulation_targets.engines[side].target_direction_mode_changed = 0;

	    g_regulation_targets.engines[side].abs_target_velocity_mps = velocity_mps > 0 ? velocity_mps : 0;
	    g_regulation_targets.engines[side].target_direction_mode = target_mode;
	    g_regulation_targets.engines[side].use_regulator_break = use_break;
	    g_regulation_targets.engines[side].break_threshold_mps = -fabsf(break_threshold);
	    g_regulation_targets.engines[side].active_break = 0;
	}

	g_differential_active = 1;
	g_differential_target_velocity_mps = velocity_mps;
}

/**
 * Accelerates the car to the given velocity using a PID-Regulator.
 *
 * @param side
 * 		the side of the engine to regulate
 * @param velocity_mps
 * 		the target velocity
 * @param use_break
 * 		should the regulator use the break mode of the engine
 * @param break_threshold
 * 		the threshold of velocity deviation at which the break will be activated (don't care about the sign, handled automatically)
 */
void drive_accelerate_to_advanced(enum Side side, float velocity_mps, char use_break, float break_threshold)
{
    enum OperationMode target_mode = FORWARD_FREERUN; //velocity_mps < 0 ? BACKWARD_FREERUN : FORWARD_FREERUN;

//	if (g_regulation_targets.engines[side].target_direction_mode != target_mode)
//		g_regulation_targets.engines[side].target_direction_mode_changed = 1;

    g_regulation_targets.engines[side].target_direction_mode_changed = 0;

    g_regulation_targets.engines[side].abs_target_velocity_mps = velocity_mps > 0 ? velocity_mps : 0;
    g_regulation_targets.engines[side].target_direction_mode = target_mode;
    g_regulation_targets.engines[side].use_regulator_break = use_break;
    g_regulation_targets.engines[side].break_threshold_mps = -fabsf(break_threshold);
    g_regulation_targets.engines[side].active_break = 0;

    g_differential_active = 0;
}

void drive_active_break(enum Side side, unsigned int pulse_width)
{
    g_regulation_targets.engines[side].active_break = 1;
    g_regulation_targets.engines[side].active_break_pw = pulse_width;
}

/**
 * Accelerates the car to the given velocity using a PID-Regulator and the default break threshold.
 *
 * @param side
 * 		the side of the engine to regulate
 * @param velocity_mps
 * 		the target velocity
 * @param use_break
 * 		should the regulator use the break mode of the engine
 */
void drive_accelerate_to(enum Side side, float velocity_mps, char use_break)
{
    drive_accelerate_to_advanced (side, velocity_mps, use_break, DEFAULT_BREAK_THRESHOLD_MPS);
}

void drive_fixed_update()
{
    enum Side side;
    float velocity_factor = 500;  //2 mps * 500 = 1000 (max. Speed), declaring the max Speed
    float new_pulse_width;

    //float abs_current_velocity_mps, velocity_deviation_mps, new_pulse_width;
    /*
    if (g_differential_active)
    {
        float servoAngle_rad = angle_deg_to_rad_f(servo_get_position_angle_deg());

        if (servoAngle_rad < 0) // Right
        {
        	diff_calculate(differenzial_W, differenzial_B, fabs(servoAngle_rad), g_differential_target_velocity_mps,
        			&g_regulation_targets.engines[RIGHT].abs_target_velocity_mps, &g_regulation_targets.engines[LEFT].abs_target_velocity_mps);
        }
        else if (servoAngle_rad > 0) // Left
        {
        	diff_calculate(differenzial_W, differenzial_B, fabs(servoAngle_rad), g_differential_target_velocity_mps,
        	    			&g_regulation_targets.engines[LEFT].abs_target_velocity_mps, &g_regulation_targets.engines[RIGHT].abs_target_velocity_mps);
        }
    }*/

    for (side = RIGHT; side <= LEFT; side++)
    {
    	//Test des PWM-Geschwindigkeits-Verhältnisses
    	enginge_set_mode(side, FORWARD_FREERUN);
    	new_pulse_width = g_regulation_targets.engines[side].abs_target_velocity_mps*velocity_factor;
    	enginge_set_pulse_width_pm(side,new_pulse_width);




    	/*abs_current_velocity_mps = tachometer_get_velocity_mps (side == LEFT ? BACK_LEFT : BACK_RIGHT);

        abs_current_velocity_mps =
                abs_current_velocity_mps > MAX_CAR_VELOCITY_ABS_MPS ? MAX_CAR_VELOCITY_ABS_MPS : abs_current_velocity_mps;

    	// Calculate velocity deviation and add the velocity deviation to the sum for regulation purposes
        velocity_deviation_mps = g_regulation_targets.engines[side].abs_target_velocity_mps
                - abs_current_velocity_mps;

        // recalculate regulator parameters (formula provided by Micheal Cieslak)
//        float velocity_percentage = percentage_f(abs_current_velocity_mps, MAX_CAR_VELOCITY_ABS_MPS);

//        regulator_set_V(&g_regulation_targets.engines[side].regulator,
//        		-0.01637 * velocity_percentage + 3.168);
//
//        regulator_set_ti_inverse(&g_regulation_targets.engines[side].regulator,
//        		0.2853 * expf(velocity_percentage * 0.03195));

        // Calculate new pulse width, PID - Regulator
        new_pulse_width = regulator_calculate_value (&g_regulation_targets.engines[side].regulator,
        		velocity_deviation_mps) * VELOCITY_REGULATOR_OUTPUT_SCALE_FACTOR;

        if (new_pulse_width < 0)
        	new_pulse_width = 0;

        // velocity is near zero -> we can ignore a target direction mode change
        if (abs_current_velocity_mps < VELOCITY_TO_ZERO_TOLERANCE_MPS)
            g_regulation_targets.engines[side].target_direction_mode_changed = 0;

        // We are driving in the desired direction -> Regulator can operate
        if (!g_regulation_targets.engines[side].target_direction_mode_changed)
        {
            // Should the regulator use the break
            if (g_regulation_targets.engines[side].active_break)
            {
                engine_set_mode (side, BACKWARD_BREAK);
                engine_set_pulse_width_pm (side, g_regulation_targets.engines[side].active_break_pw);
            }
            else if (new_pulse_width == 0 /*|| (g_regulation_targets.engines[side].use_regulator_break
                    && velocity_deviation_mps < g_regulation_targets.engines[side].break_threshold_mps)*/)
            /*{
                engine_set_mode (side, BREAK);
            }
            else
            {
                engine_set_pulse_width_pm (side, (unsigned int) new_pulse_width);
                engine_set_mode (side, g_regulation_targets.engines[side].target_direction_mode);
            }
        }
        else // Break down, because the target direction has been changed
        {
            engine_set_mode (side, BREAK);
        }*/
    }
}

