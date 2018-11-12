/*
 * global_line_buffer.c
 *
 *  Created on: Mar 11, 2017
 *      Author: Tobias Hupel
 *
 *  Global Line Buffer suitable for most general purposes.
 */

#include "line_buffer.h"
#include "Ir_Board.h"
#include "distance_alarm.h"

//#define DEBUG_LINE_BUFFER

#define LINE_BUFFER_RESOLUTION 0.001f

struct LineBuffer g_line_buffer;
struct DistanceAlarm g_buffer_update_alarm;

/**
 * Calculates the gradient of average positions of ones over the given length of the Global Line Buffer.
 *
 * @param length
 *      the length
 * @return
 *      the gradient of average positions
 */
float global_line_buffer_calculate_gradient(unsigned int length)
{
    return line_buffer_calculate_gradient (&g_line_buffer, length);
}

/**
 * Calculates the average of average positions of ones over the given length of the Global Line Buffer.
 *
 * @param length
 *      the length
 * @return
 *      the average of average positions
 */
float global_line_buffer_calculate_average(unsigned int length)
{
    return line_buffer_calculate_average (&g_line_buffer, length);
}

/**
 * Initializes the Global Line Buffer.
 */
void global_line_buffer_initialize()
{
    line_buffer_new (&g_line_buffer);
    distance_alarm_new_mean (&g_buffer_update_alarm, LINE_BUFFER_RESOLUTION, 1);
}

/**
 * Updates the Global Line Buffer.
 */
void global_line_buffer_update()
{
    if (distance_alarm_has_distance_reached (&g_buffer_update_alarm))
    {
        distance_alarm_reset (&g_buffer_update_alarm);

        line_buffer_add_pattern (&g_line_buffer, ir_get_pattern ());

#ifdef DEBUG_LINE_BUFFER
#include "serial_blue.h"
#include <stdio.h>

#define PATTERN_COUNT 20

        static char writeBuffer[64], patternCount = 0;

        patternCount++;

        if (patternCount >= PATTERN_COUNT)
        {
            sprintf (writeBuffer, "20cm grd: %1.6f av: %1.6f\n",
                     global_line_buffer_calculate_gradient (PATTERN_COUNT),
                     global_line_buffer_calculate_average (PATTERN_COUNT));

            serial_blue_write_string(writeBuffer);

            patternCount = 0;
        }

#endif
    }
}
