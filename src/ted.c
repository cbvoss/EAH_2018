/*
 * ted.c
 *
 *  Created on: 04.01.2017
 *      Author: Florian Schaedlich
 *
 *  Reviewed by: Florian Schaedlich
 *  			 Adrian Zentgraf
 *  			 Franz Lübke
 */
#include "config_car.h"
#include "Ir_Board.h"
#include "ted.h"
#include "distance_alarm.h"
#include "tachometer.h"
#include "serial_blue.h"
#include "TEDnew.h"
#include "servo.h"
#include <math.h>
#include <stdio.h>
#include "opto_interruptor.h"
#include "TED_Version_3.h"

/*
 * Effects of the different defines:
 * deadlock --> amount of millimeter between detecting an event and and setting it as the actual event
 * resolution --> how often the front panel is scanned
 * border_min --> minimum width of line
 * border_max --> maximum width of line
 *
 * borders should be a multiple of the resolution
 */

//#define old_ted
//#define new_ted
#define TED_VERSION_3
//#define DEBUG_Track_events

#define RESOLUTION_mm 1
#define TED_MAX_DETECTION_WORK_ANGLE_ABS 10.0f

float g_predicted_event_m = 0;
float g_delta_distance_left = 0.0f;
float g_delta_distance_right = 0.0f;
float g_pattern[3][2];
char g_actual_left = 0;
char g_actual_right = 0;
enum track_event g_possible_event = NONE;
enum track_event g_current_event = NONE;

struct DistanceAlarm g_ted_distance_alarm;

/**
 * Set Event to NONE
 * Necessary for detecting new events
 */
void ted_reset_track_event()
{
#ifdef old_ted
    g_possible_event = NONE;
    g_current_event = NONE;

    int i;
    int j;
    for (i = 0; i < 3; i++)
    {
        for (j = 0; j < 2; j++)
        {
            g_pattern[i][j] = 0.0f;
        }
    }
#endif

#ifdef new_ted
    TEDnew_reset_picture();
    TEDnew_activate();
#endif
}

/**
 * create distance alarm and initialize buffer to zero
 */
void ted_initialize()
{
#ifdef new_ted
	TEDnew_picture_initialize();
#endif
#ifdef old_ted
    ted_reset_track_event ();
#endif
#ifdef TED_VERSION_3
    Ted_Version_3_Initialize();
#endif
}

/**
 * prove if the race started
 * @return start_enable
 * if start_enable: zero --> no start was detected
 * 					one  --> start is detected
 * */
int ted_startbar_detection()
{
    if (ir_get_value (0) == 1)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void ted_encript(char side)
{
    int predicted_event = 1;
    int i;

    for (i = 0; i < 3 && predicted_event != 0; i++)
    {
        switch (i)
        {
            case (0):
                if (((((float) BORDER_WHITE_MIN_mm) / 1000.0f) <= g_pattern[i][side])
                        && ((((float) BORDER_WHITE_MAX_mm) / 1000.0f) >= g_pattern[i][side]))
                {}
                else
                {
                    predicted_event = 0;
                }
            break;
            case (1):
                if (((((float) BORDER_BLACK_MIN_mm) / 1000.0f) <= g_pattern[i][side])
                        && ((((float) BORDER_BLACK_MAX_mm) / 1000.0f) >= g_pattern[i][side]))
                {}
                else
                {
                    predicted_event = 0;
                }
            break;
            case (2):
                if (((((float) BORDER_WHITE_MIN_mm) / 1000.0f) <= g_pattern[i][side])
                        && ((( 120.0f) / 1000.0f) >= g_pattern[i][side]))
                {}
                else
                {
                    predicted_event = 0;
                }
            break;
        }
    }
    // event detected
    if ((predicted_event == 1) && (side == 0))
    {
        if (g_possible_event == JUMP_RIGHT)
        {
            g_possible_event = SQUARE;

#ifdef DEBUG_ted
            char string[40];
            snprintf (string, 40, "SQUARE\n\r");
            serial_blue_write_string (string);
#endif
        }
        else
        {
            g_possible_event = JUMP_LEFT;
            g_predicted_event_m = tachometer_get_extrapolated_arithemtic_mean_distance_meter ();

#ifdef DEBUG_ted
            char string[40];
            snprintf (string, 40, "JUMP_LEFT\n\r");
            serial_blue_write_string (string);
#endif
        }
    }
    if ((predicted_event == 1) && (side == 1))
    {
        if (g_possible_event == JUMP_LEFT)
        {
            g_possible_event = SQUARE;
#ifdef DEBUG_ted
            char string[40];
            snprintf (string, 40, "SQUARE\n\r");
            serial_blue_write_string (string);
#endif
        }
        else
        {
            g_possible_event = JUMP_RIGHT;
            g_predicted_event_m = tachometer_get_extrapolated_arithemtic_mean_distance_meter ();

#ifdef DEBUG_ted
            char string[40];
            snprintf (string, 40, "JUMP_RIGHT\n\r");
            serial_blue_write_string (string);
#endif
        }
    }
}

/**
 * Discern Track_Events
 *
 * possible events: NONE, JUMP_LEFT, JUMP_RIGHT, SQUARE
 * The module wait for deadlock before it switch detected events to actual events - ?
 *
 * The current event can only be reseted with the reset function; !!!There is no opportunity to detect a new event
 * without reseting!!!
 */
void ted_update()
{
#ifdef new_ted
	TEDnew_Update();
#endif
#ifdef old_ted
    if (g_current_event == NONE && fabsf(servo_get_position_angle_deg()) < TED_MAX_DETECTION_WORK_ANGLE_ABS)
    {
        int i;
        // left-side pattern
        if ((ir_get_value (7) != g_actual_left) && g_possible_event != SQUARE && g_possible_event != JUMP_LEFT)
        {
            g_actual_left = ir_get_value (7);

            for (i = 0; i < 2; i++)
            {
                g_pattern[2 - i][0] = g_pattern[1 - i][0];
            }

            g_pattern[0][0] = tachometer_get_extrapolated_arithemtic_mean_distance_meter () - g_delta_distance_left;
            g_delta_distance_left = tachometer_get_extrapolated_arithemtic_mean_distance_meter ();

            if (g_actual_left == 0)
            {
                ted_encript (0);
            }

#ifdef DEBUG_ted
            char string[128];
            sprintf (string, "Left: %d;%d;%d;%d\n", g_actual_left, (int)(g_pattern[0][0] * 1000), (int)(g_pattern[1][0] * 1000),
                      (int)(g_pattern[2][0] * 1000));
            serial_blue_write_string (string);
#endif
        }
        // right-side pattern
        if (ir_get_value (0) != g_actual_right && g_possible_event != SQUARE && g_possible_event != JUMP_RIGHT)
        {
            g_actual_right = ir_get_value (0);

            for (i = 0; i < 2; i++)
            {
                g_pattern[2 - i][1] = g_pattern[1 - i][1];
            }

            g_pattern[0][1] = tachometer_get_extrapolated_arithemtic_mean_distance_meter () - g_delta_distance_right;
            g_delta_distance_right = tachometer_get_extrapolated_arithemtic_mean_distance_meter ();

            if (g_actual_right == 0)
            {
                ted_encript (1);
            }

#ifdef DEBUG_ted
            char string[128];
            sprintf (string, "Right: %d;%d;%d;%d\n", g_actual_right, (int)(g_pattern[0][1] * 1000), (int)(g_pattern[1][1] * 1000),
                      (int)(g_pattern[2][1] * 1000));
            serial_blue_write_string (string);
#endif
        }

        if ((g_possible_event != NONE)
                && ((tachometer_get_extrapolated_arithemtic_mean_distance_meter () - g_predicted_event_m)
                        >= (((float) DEADLOCK_mm) / 1000.0f)))
        {
            g_current_event = g_possible_event;
        }

    }
#endif

#ifdef TED_VERSION_3
    TED3_update();
#endif

}

/**
 *
 * @return current track event (active event)
 */
enum track_event ted_get_track_event()
{
#ifdef DEBUG_Track_events
    char string[40];
    switch (TEDnew_get_track_event())	{
    case NONE:
    	return NONE;

    case SQUARE:
    	snprintf (string, 40, "SQUARE\n");
    	break;

    case JUMP_LEFT:
    	snprintf (string, 40, "JUMP_LEFT\n");
    	break;

    case JUMP_RIGHT:
    	snprintf (string, 40, "JUMP_RIGHT\n");
    	break;

    default:
    	return NONE;
    }
    TEDnew_reset_picture();
    serial_blue_write_string (string);
    return NONE;
#endif

#ifdef new_ted
    if(TEDnew_get_track_event() != NONE)	TEDnew_deactivate();
	return TEDnew_get_track_event();
#endif

#ifdef old_ted
    return g_current_event;
#endif
    return 0;
}

#ifdef new_ted
int ted_get_score(enum Side side)
{
	return TEDnew_get_score(side);
}

void ted_reset_picture()	{
	TEDnew_reset_picture();
}

#endif

