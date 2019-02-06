/*
 * MCURally.c
 *
 *  Created on: Nov 26, 2016
 *      Author: Tobias Hupel
 */
#include "config_car.h"
#include "iodefine.h"
#include "vect.h"
#include "controller_jump.h"
#include "controller_square.h"
#include "serial_com.h"
#include "engine.h"
#include "servo.h"
#include "TED_Version_3.h"
#include "ted.h"
#include "controller_hold_line.h"
#include "adc.h"
#include "serial_blue.h"
#include "global_clock.h"
#include "tachometer.h"
#include "distance_alarm.h"
#include "drive.h"
#include "drive_curve.h"
#include "event_map.h"
#include "switches.h"
#include "Ir_Board.h"
#include "global_line_buffer.h"
#include "fuzzy.h"
#include "debug_routines.h"
#include "opto_interruptor.h"
#include "opto_interruptor.h"
#include "spi.h"
#include "TEDnew.h"


#ifdef DEBUG
#include "data_log.h"
#include <stdio.h>

struct timer g_log_timer;
#endif

#define DRIVE_SPEED_MPS 1.5f
//#define USE_FUZZY_TARGET_SPEED


#define FIXED_UPDATE_PERIOD_LENGTH 39					// 0,1016ms at PCLK / 128 and PCLK = 49.152MHz
struct timer g_ms_tick;

#ifdef DEBUG_mm_tick
struct DistanceAlarm g_mm_tick;
#endif

/**
 * Global initialization
 */
void initialize()
{
    // System Clock
    SYSTEM.SCKCR.BIT.ICK = 1;               			// 12.288*4 = 49.152MHz
    SYSTEM.SCKCR.BIT.PCK = 1;               			// 12.288*4 = 49.152MHz
    global_clock_set_100us_timer (&g_ms_tick, 10);

#ifdef DEBUG
    global_clock_set_timer (&g_log_timer, 3);
    PORTA.DDR.BIT.B0 = 1;
    PORTA.DDR.BIT.B1 = 1;
    PORTA.DDR.BIT.B2 = 1;
    PORTA.DDR.BIT.B3 = 1;
    PORTA.DR.BIT.B0 = 1;
    PORTA.DR.BIT.B1 = 1;
    PORTA.DR.BIT.B2 = 1;
    PORTA.DR.BIT.B3 = 1;
#endif
#ifdef DEBUG_mm_tick
    distance_alarm_new_specific(&g_mm_tick, 0.008f, 0 ,BACK_LEFT);
#endif

}

/**
 * Initializes the fixed update routine using CMT0 and Compare Match Interrupt.
 */
void initialize_fixed_update()
{
    SYSTEM.MSTPCRA.BIT.MSTPA15 = 0;						// enable Compare Match Timer (Unit 0)

    CMT.CMSTR0.BIT.STR0 = 0;							// stop counting on CMT0

    CMT0.CMCR.BIT.CKS = 2;								// Prescaler: PCLK / 128
    CMT0.CMCR.BIT.CMIE = 1;								// enable Compare Match Interrupt
    CMT0.CMCOR = FIXED_UPDATE_PERIOD_LENGTH;			// max count value, compare match value
    CMT0.CMCNT = 0;										// initial count value = 0

    ICU.IER[0x03].BIT.IEN4 = 1;							// Interrupt Request Enable
    ICU.IPR[0x04].BIT.IPR = 7;							// Interrupt Priority

    CMT.CMSTR0.BIT.STR0 = 1;							// start counting on CMT0
}

/**
 * Initializes all modules.
 */
void initialize_modules()
{
    // place your module initialization functions here
    ir_initialize ();
    serial_com_initialize ();
    engine_initialize ();
    opto_initialize ();
    servo_initialize ();
    servo_set_borders (SERVO_LEFT, SERVO_CENTER, SERVO_RIGHT);
    drive_initialize ();
    Ted_Version_3_Initialize();
    serial_blue_initialize ();
    tachometer_initialize ();
    adc_initialize ();
    hold_line_initalize ();
    drive_curve_initialize ();
    controller_square_initialize ();
    event_map_Initialize ();
    switches_initialize ();
    global_line_buffer_initialize ();
    fuzzy_initialize();
}

/**
 * Continuous update for all modules.
 */
void update_modules()
{
    // place your module update functions here
    tachometer_update ();
    hold_line_update ();
    TED3_update();
    drive_curve_update ();
    controller_jump_update ();
    controller_square_update ();
    global_line_buffer_update ();
    fuzzy_update();
    }

/**
 * @deprecated the call interval isn't accurate enough, please use the timer functionality instead
 *
 * Continuous update for all modules in a certain and fixed time step (1ms).
 */
void update_modules_fixed()
{
    static int zehn_ms = 100;
    float distance = 0;

	// place your module fixed update functions here
	drive_curve_update_fixed();
    //drive_fixed_update ();
    //Für Test der PWM
	enum Side side;
	float new_pulse_width;
    zehn_ms--;
    if (zehn_ms <= 0){
    	zehn_ms = 100;
    	debug_ticks();
    	distance = tachometer_get_distance_meter(BACK_RIGHT);
    	if (distance >= 0 && distance < 2){
    		for (side = RIGHT; side <= LEFT; side++)
    		    {
    		    	engine_set_mode(side, FORWARD_FREERUN);
    		    	new_pulse_width = 500;		//Hier den Wert für die Geschwindigkeit setzten, 0=Stillstand, 1000=max Speed
    		    	engine_set_pulse_width_pm(side,new_pulse_width);
    		    }
    	}
    	else{
    		for (side = RIGHT; side <= LEFT; side++)
    		    		    {
    		    		    	engine_set_mode(side, FORWARD_FREERUN);
    		    		    	new_pulse_width = 0;		//Hier den Wert für die Geschwindigkeit setzten, 0=Stillstand, 1000=max Speed
    		    		    	engine_set_pulse_width_pm(side,new_pulse_width);
    		    		    }
    	}
    }

}

/**
 * Logs the current front pattern. Only changes in front pattern will be logged.
 **/
void debugFrontPattern()
{
#ifdef DEBUG_FrontPattern
    static char initDone = 0;
    static float lastDist = 0;
    static char lastPattern = 0;
    static char lastWasWritten = 0;
    float dist;
    int pattern;
    char buffer[32];

    if (!initDone)
    {
        initDone = 1;
        lastDist = tachometer_get_distance_meter(BACK_RIGHT);
        lastPattern = ir_get_pattern ();
    }

    dist = tachometer_get_extrapolated_distance_meter(BACK_RIGHT);
    if (floatAbs (dist - lastDist) < 0.001f)
    return;

    pattern = ir_get_pattern ();

    if (pattern == lastPattern)
    {
        lastDist = dist;
        lastWasWritten = 0;
        return;
    }

    if (lastWasWritten == 0)
    {
        snprintf (buffer, 32, "%2.3f\t%d\n", lastDist, lastPattern);
        serial_blue_write_string (buffer);
    }

    snprintf (buffer, 32, "%2.3f\t%d\n", dist, pattern);
    serial_blue_write_string (buffer);
    lastWasWritten = 1;
    lastPattern = pattern;
    lastDist = dist;
#endif
}

/**
 * Updates the event map with the current track event, if it changed.
 * @param last_track_event
 * 	The latest detected track event.
 * @param current_track_event
 * 	The currently active track event.
 */
void update_eventMap_if_necessary(enum track_event last_track_event, enum track_event current_track_event)
{
    float dist;
    if (last_track_event == current_track_event)
        return;

    dist = tachometer_get_distance_meter (BACK_RIGHT);
    event_map_update (dist, current_track_event);

#if defined(DEBUG_event_Map) || defined(DEBUG_FrontPattern)
    char eventChar;
    char buffer[64];

    switch (current_track_event)
    {
        case (SQUARE):
        eventChar = 'Q';
        break;
        case (JUMP_LEFT):
        eventChar = 'L';
        break;
        case (JUMP_RIGHT):
        eventChar = 'R';
        break;
        default:
        eventChar = '?';
        break;
    }
    snprintf(buffer, 64, "update map %c\n", eventChar);
    serial_blue_write_string(buffer);
#endif
}

/**
 * Logs the current event map state.
 */
void debugEventMapState()
{
#ifdef DEBUG_event_Map
    static struct timer eventUpdate;
    static char initialized = 0;
    char buffer[64];
    enum SyncStates state;
    char syncChar;
    float v, pos;

    if (initialized == 0)
    {
        initialized = 1;
        global_clock_set_timer(&eventUpdate, 10);
    }

    if (!global_clock_timer(&eventUpdate))
    return;

    global_clock_reset_timer(&eventUpdate);

    pos = tachometer_get_arithemtic_mean_distance_meter();
    v = event_map_get_velocity_recomendation(pos);
    state = event_map_get_sync_State();
    switch (state)
    {
        case SYNC: syncChar = 'S'; break;
        case PARTIALLY_LOST: syncChar = 'P'; break;
        case FULLY_LOST: syncChar = 'L'; break;
    }

    snprintf(buffer, 64, "s=%0.3f v=%0.3f syn=%c\n", pos, v, syncChar);
    serial_blue_write_string(buffer);

#endif
}

/**
 * Checks whether the user signalized that we fell from the map and tells the event map, if necessary.
 */
void check_UserMapLost()
{
    if (!switches_get_MotorDriveBoardSwitch ())
        return;

    event_map_Indicate_SyncLost (1);
}

void checkStartBar()
{
	while (ir_get_front_sensor())
	{
	}
}

/**
 * Starts the MCU Rally Control Logic.
 */
void mcu_rally_main()
{
    initialize ();
    initialize_modules ();
    initialize_fixed_update ();

    //checkStartBar();

    drive_accelerate_to_differential(DRIVE_SPEED_MPS, 0);

    while (1)
    {

#ifdef DEBUG_DRIVE_REGULATOR
    	debug_drive_regulator('s', 2000, 4000, 6000, 50, 1.5f, 1.0f, 0.0f, 0, 0.1f);
#endif
#if defined(DEBUG_data_log) || defined(DEBUG_remote)
        static char stop = 1;
        char res = serial_blue_pull_receive_buffer ();
        if (res == 's'){
        	stop = 0;
        	drive_accelerate_to(RIGHT, 0.5f, 0);
        	drive_accelerate_to(LEFT, 0.5f, 0);
        }
        if (res == 'q'){
        	stop = 1;
        	drive_accelerate_to(RIGHT, 0.0f, 1);
        	drive_accelerate_to(LEFT, 0.0f, 1);
        }
        if (!stop)
        {

#ifdef DEBUG_data_log
            data_log(&g_log_timer,0);
#endif
#endif


        static enum track_event last_track_event = NONE;
        enum track_event current_track_event = NONE;
		
        static float targetVelocity_mps = DRIVE_SPEED_MPS;

        current_track_event = NONE; //ted_get_track_event (); -> NONE ist für den Test der PWM

#ifdef ENABLE_CONTROLLER_STATE_MACHINE
        switch (current_track_event)
        {
			case SQUARE:
				if (last_track_event == NONE)
					drive_curve_set_active (0);
				controller_square_set_active (1);
				if (controller_square_has_finished ())
				{
					controller_square_set_active (0);
					ted_reset_track_event ();
					//controller_square_reset ();
					hold_line_set_active (0);
					drive_curve_set_active (0);
				}
            break;
            case JUMP_LEFT:
                if (last_track_event == NONE)
                    drive_curve_set_active (0);
                controller_jump_set_jump_side (LEFT);
                controller_jump_set_active (1);
                if (controller_jump_has_finished ())
                {
                    ted_reset_track_event ();
                    hold_line_set_active (0);
                    controller_jump_reset ();
                    drive_curve_set_active (0);
                }
            break;
            case JUMP_RIGHT:
                if (last_track_event == NONE)
                    drive_curve_set_active (0);
                controller_jump_set_jump_side (RIGHT);
                controller_jump_set_active (1);
                if (controller_jump_has_finished ())
                {
                    ted_reset_track_event ();
                    hold_line_set_active (0);
                    controller_jump_reset ();
                    drive_curve_set_active (0);
                }
            break;
            case NONE:
                //debugEventMapState ();

#if defined(USE_FUZZY_TARGET_SPEED)
                targetVelocity_mps = fuzzy_calculate_target_velocity_mps(ir_get_pattern(), servo_get_position_angle_deg(), targetVelocity_mps);
#endif
                //drive_curve_set_max_speed (targetVelocity_mps);
                //drive_curve_set_active (1);
            break;
        }
#endif
        debugFrontPattern ();
        update_eventMap_if_necessary (last_track_event, current_track_event);
        last_track_event = current_track_event;

#if defined(DEBUG_data_log) || defined(DEBUG_remote)
    }
    else
    {
        drive_accelerate_to (LEFT, 0.0f, 1);
        drive_accelerate_to (RIGHT, 0.0f, 1);

        drive_curve_set_active(0);
        controller_square_set_active(0);
        controller_jump_set_active(0);
    }
#endif

        // Update Modules
        update_modules ();

        // fixed
        if (global_clock_timer (&g_ms_tick))
        {
        	global_clock_reset_timer (&g_ms_tick);
        	update_modules_fixed ();

        }
    }
}

/**
 * CMT0 Compare Match Interrupt
 *
 * Triggers fixed update
 */
void Excep_CMT0_CMI0(void)
{
    global_clock_fixed_update ();
#ifdef DEBUG_us
    PORTA.DR.BIT.B0^=1;
#endif

}
