/*
 * tachometer.c
 *
 *	This module transforms the opto sensor information to data with standard units.
 *
 *	Features:
 *		- velocity in meters per second and rotations per second
 *		- distance in meter since startup
 *
 *  Created on: 02.12.2016
 *
 *      Author: Tobias Hupel
 *      		Eric Elsner
 *
 *      Reviewed on 02.12.2016 by:
 *      		Tobias Hupel
 *      		Eric Elsner
 *      		Adrian Zentgraf
 *
 *      Extension Reviewed on 26.01.2017 by:
 *      		Franz Lübke
 *      		Tobias Hupel
 */

#include "global_clock.h"
#include "enums.h"
#include "config_car.h"
#include "math_tools.h"
#include "opto_interruptor.h"

// TODO: figure out the correct wheel radius

#define ICLK 49152000.0f			// System Clock

#define WATCH_TIME_INTERVAL 500  // 500ms

struct VelocityWatchDog
{
    struct timer watch_dog_timer[2];
    unsigned int last_tick_count[2];
    unsigned int tick_count_changed_time_100us[2];
    char is_wheel_stuck[2];
} g_velocity_watch_dog;

/**
 * Updates the velocity watchdog
 *
 * @param side
 *      the side to update
 */
void update_velocity_watchdog(enum Side side)
{
    unsigned int current_tick_count = opto_get_tick_count (side == RIGHT ? BACK_RIGHT : BACK_LEFT);

    if (current_tick_count != g_velocity_watch_dog.last_tick_count[side])
    {
        g_velocity_watch_dog.tick_count_changed_time_100us[side] = global_clock_get_100us_count ();

        g_velocity_watch_dog.is_wheel_stuck[side] = 0;
        g_velocity_watch_dog.last_tick_count[side] = current_tick_count;

        global_clock_reset_timer (&g_velocity_watch_dog.watch_dog_timer[side]);
    }
    else if (global_clock_timer (&g_velocity_watch_dog.watch_dog_timer[side]))
    {
        g_velocity_watch_dog.is_wheel_stuck[side] = 1;
    }
}

/**
 *	Calculates the distance in meter between two magnets of a wheel.
 *
 * @return
 * 		the distance between in meter
 */
float get_distance_between_magnet_ticks_m(enum wheel_selector side)
{
    return (side == BACK_LEFT) ? DISTANCE_BETWEEN_MAGNET_TICKS_LEFT_M : DISTANCE_BETWEEN_MAGNET_TICKS_RIGHT_M;
}

/**
 * Calculates the time in seconds between tow "opto ticks" of a wheel.
 *
 * @param wheel
 * 		the wheel of interest
 * @return
 * 		the time in seconds
 */
float get_time_between_opto_ticks_seconds(enum wheel_selector wheel)
{
    return ((float) opto_get_counts_per_tick(wheel, OPTO_COUNTS_PER_TICK_BUFFER_SIZE))
    		/ (ICLK / opto_get_counter_prescaler ());
}

/**
 * Calculates the velocity in meter per seconds of the desired wheel.
 *
 * @param wheel
 * 		the wheel of interest
 * @return
 * 		the velocity in meter per seconds
 */
float tachometer_get_velocity_mps(enum wheel_selector wheel)
{
    static float velocity = 0.0f;
    float tick_time = get_time_between_opto_ticks_seconds (wheel);

    if (tick_time > 0.0f)
    {
        if (g_velocity_watch_dog.is_wheel_stuck[(wheel == BACK_LEFT) ? LEFT : RIGHT])
            velocity = 0.0f;
        else
            velocity = get_distance_between_magnet_ticks_m (wheel) / tick_time;
    }

    return velocity;
}

/**
 * Calculates the distance in meters since startup.
 *
 * @param wheel
 * 		the wheel of interest
 * @return
 * 		the distance in meter
 */
float tachometer_get_distance_meter(enum wheel_selector wheel)
{
    return opto_get_tick_count (wheel) * get_distance_between_magnet_ticks_m (wheel);
}

/**
 * Calculates the arithmetic mean distance of the BACK_LEFT wheel distance and the BACK_RIGHT wheel distance
 *
 * @return
 * 		the arithmetic mean distance in meter
 */
float tachometer_get_arithemtic_mean_distance_meter()
{
    return (tachometer_get_distance_meter (BACK_RIGHT) + tachometer_get_distance_meter (BACK_LEFT)) / 2.0f;
}

/**
 * Calculates an extrapolated distance in meters since startup based on the last measured velocity.
 *
 * @param wheel
 * 		the wheel of interest
 * @return
 * 		the extrapolated distance in meter
 */
float tachometer_get_extrapolated_distance_meter(enum wheel_selector wheel)
{
    enum Side side = wheel == BACK_RIGHT ? RIGHT : LEFT;
    float deltaTimeSeconds, extrapolatedDistance, distanceBetweenMagnets;

    update_velocity_watchdog (side);

    deltaTimeSeconds = ((float) (global_clock_get_100us_count ()
            - g_velocity_watch_dog.tick_count_changed_time_100us[side])) / 10000.0f;

    extrapolatedDistance = tachometer_get_velocity_mps (wheel) * deltaTimeSeconds;
    distanceBetweenMagnets = get_distance_between_magnet_ticks_m(wheel);

    if (extrapolatedDistance > distanceBetweenMagnets)
        extrapolatedDistance = distanceBetweenMagnets;

    return tachometer_get_distance_meter (wheel) + extrapolatedDistance;
}

/**
 * Calculates the arithmetic mean distance of the BACK_LEFT wheel extrapolated distance and the BACK_RIGHT wheel extrapolated distance
 *
 * @return
 * 		the extrapolated arithmetic mean distance in meter
 */
float tachometer_get_extrapolated_arithemtic_mean_distance_meter()
{
    return (tachometer_get_extrapolated_distance_meter (BACK_RIGHT)
            + tachometer_get_extrapolated_distance_meter (BACK_LEFT)) / 2.0f;
}

/**
 * Calculates the arithmetic mean velocity of the BACK_LEFT wheel velocity and the BACK_RIGHT wheel velocity
 *
 * @return
 * 		the arithmetic mean velocity in meter per second
 */
float tachometer_get_arithemtic_mean_velocity_mps()
{
    return (tachometer_get_velocity_mps (BACK_LEFT) + tachometer_get_velocity_mps (BACK_RIGHT)) / 2.0f;
}

void tachometer_initialize()
{
    global_clock_set_timer (&g_velocity_watch_dog.watch_dog_timer[RIGHT],
    WATCH_TIME_INTERVAL);
    global_clock_set_timer (&g_velocity_watch_dog.watch_dog_timer[LEFT],
    WATCH_TIME_INTERVAL);
    g_velocity_watch_dog.last_tick_count[RIGHT] = g_velocity_watch_dog.last_tick_count[LEFT] = 0;
    g_velocity_watch_dog.tick_count_changed_time_100us[RIGHT] =
            g_velocity_watch_dog.tick_count_changed_time_100us[LEFT] = 0;
    g_velocity_watch_dog.is_wheel_stuck[RIGHT] = g_velocity_watch_dog.is_wheel_stuck[LEFT] = 0;
}

void tachometer_update()
{
    for (enum Side side = RIGHT; side <= LEFT; side++)
    {
        update_velocity_watchdog (side);
    }
}

