/*
 * config_car.h
 *
 *  Created on: 23.02.2017
 *      Author: franz_2
 */

#ifndef CONFIG_CAR_H_
#define CONFIG_CAR_H_
//Define which CAR is used
#define CAR1
//#define CAR2
//#define CAR3

#define ENABLE_CONTROLLER_STATE_MACHINE

/**
 * DEBUG Options
 */
//#define DEBUG
//#define DEBUG_STATE
//#define DEBUG_data_log
//#define DEBUG_ted
//#define DEBUG_remote
//#define DEBUG_mm_tick
//#define DEBUG_us
//#define DEBUG_fix_time
//#define DEBUG_main_time
//#define DEBUG_event_Map
//#define DEBUG_FrontPattern
//#define DEBUG_LINE_BUFFER
//#define DEBUG_DRIVE_REGULATOR
//#define DEBUG_binary_picture
//#define DEBUG_ted_threshhold
//END DEBUG OPTIONS

/**
* calibration section
 */

#define MAX_CAR_VELOCITY_ABS_MPS 4.0f

#ifdef CAR1
	#define SUB_REG AD0.ADDRG
	#define LEFT_IR_REG AD0.ADDRH
	#define RIGHT_IR_REG AD0.ADDRI
	#define DISTANCE_BETWEEN_MAGNET_TICKS_RIGHT_M 0.002f
	#define DISTANCE_BETWEEN_MAGNET_TICKS_LEFT_M  0.002f
	/**Calibrate Servo
	 */
	#define SERVO_LEFT  	57605 // Real: 63645
	#define SERVO_CENTER 	60645
	#define SERVO_RIGHT 	63565 // Real: 57685

	#define MAX_SERVO_ANGLE_LEFT_ABS_DEG 90.0f
	#define MAX_SERVO_ANGLE_RIGHT_ABS_DEG 90.0f

/**
 * Calibrate IR values
 */
	#define GRAY 60
	#define WITHE 100
	#define BLACK 10
	#define IR_EPSILON 40
/**
 * Calibrate TED borders

	#define BORDER_WHITE_MIN_mm 20
	#define BORDER_WHITE_MAX_mm 35
	#define BORDER_BLACK_MIN_mm 10
	#define BORDER_BLACK_MAX_mm 30
*/
	#define DEADLOCK_mm 40
	#define BORDER_WHITE_MIN_mm 5		//8
	#define BORDER_WHITE_MAX_mm 35		//60
	#define BORDER_BLACK_MIN_mm 8		//2
	#define BORDER_BLACK_MAX_mm 45		//30
/**
 * Maße der Autos notieren (Angaben in mm)
 * W = Radstand
 * T = Breite
 * micha
 */
	#define differenzial_W 193
	#define differenzial_B 145
//Tachomter Berechnung
	#define MAGNETS_PER_WHEEL 48.0f
	#define WHEEL_RADIUS_METER 0.02812f 	// radius of a wheel

// Drive Curve Parameters
	#define SQUARE_ANGLE_LEFT 			+55.0f
	#define SQUARE_ANGLE_RIGHT 	 		-55.0f
	#define SQUARE_VELOCITY			  	  0.8f
//	#define SQUARE_DISTANCE_TO_STEER	  0.02f

#endif

#ifdef CAR2
	#define SUB_REG AD0.ADDRG
	#define LEFT_IR_REG AD0.ADDRI
	#define RIGHT_IR_REG AD0.ADDRJ
	#define DISTANCE_BETWEEN_MAGNET_TICKS_RIGHT_M 0.007362f
	#define DISTANCE_BETWEEN_MAGNET_TICKS_LEFT_M  0.007362f
/**Calibrate Servo
 */
	#define SERVO_LEFT 		58025	//64225 Real Tobias 64000
	#define SERVO_CENTER 	61125
	#define SERVO_RIGHT 	64125 	//58125 Real Tobias: 57850

	#define MAX_SERVO_ANGLE_LEFT_ABS_DEG 90.0f
	#define MAX_SERVO_ANGLE_RIGHT_ABS_DEG 90.0f
/**
 * Calibrate IR values
 */
	#define GRAY 20
	#define WITHE 40
	#define BLACK 1
	#define IR_EPSILON 20
/**
 * Calibrate TED borders

	#define DEADLOCK_mm 50
	#define BORDER_WHITE_MIN_mm 5
	#define BORDER_WHITE_MAX_mm 25
	#define BORDER_BLACK_MIN_mm 15
	#define BORDER_BLACK_MAX_mm 35
*/
	#define DEADLOCK_mm 50			//60
	#define BORDER_WHITE_MIN_mm 12	//8
	#define BORDER_WHITE_MAX_mm 35	//60
	#define BORDER_BLACK_MIN_mm 15	//2
	#define BORDER_BLACK_MAX_mm 40	//30
/**
 * Maße der Autos notieren (Angaben in mm)
 * W = Radstand
 * T = Breite
 * micha
 */
	#define differenzial_W 193
	#define differenzial_B 145

//Tachomter Berechnung
	#define MAGNETS_PER_WHEEL 48.0f
	#define WHEEL_RADIUS_METER 0.0277168f 	// radius of a wheel

// Drive Curve Parameters
	#define SQUARE_ANGLE_LEFT 			+55.0f
	#define SQUARE_ANGLE_RIGHT 	 		-55.0f
	#define SQUARE_VELOCITY			  	  0.65f
	#define SQUARE_DISTANCE_TO_STEER	  0.02f



#endif

#ifdef CAR3
	#define SUB_REG AD0.ADDRG
	#define LEFT_IR_REG AD0.ADDRI
	#define RIGHT_IR_REG AD0.ADDRJ
	#define DISTANCE_BETWEEN_MAGNET_TICKS_RIGHT_M 0.021333f
	#define DISTANCE_BETWEEN_MAGNET_TICKS_LEFT_M  0.016667f
/**
 * Calibrate Servo
 */
	#define SERVO_LEFT 		61850
	#define SERVO_CENTER 	60850
	#define SERVO_RIGHT 	59800

	#define MAX_SERVO_ANGLE_LEFT_ABS_DEG 59.0f		//maximal angle for side, dummy
	#define MAX_SERVO_ANGLE_RIGHT_ABS_DEG 59.0f		//maximal angle for side, dummy

/**
 * Calibrate IR values
 */
	#define GRAY 20
	#define WITHE 40
	#define BLACK 1
	#define IR_EPSILON 20
/**
 * Calibrate TED borders

	#define DEADLOCK_mm 50
	#define BORDER_WHITE_MIN_mm 5
	#define BORDER_WHITE_MAX_mm 25
	#define BORDER_BLACK_MIN_mm 15
	#define BORDER_BLACK_MAX_mm 35
*/
	#define DEADLOCK_mm 60
	#define BORDER_WHITE_MIN_mm 8
	#define BORDER_WHITE_MAX_mm 60
	#define BORDER_BLACK_MIN_mm 2
	#define BORDER_BLACK_MAX_mm 30
/**
 * Maße der Autos notieren (Angaben in mm)
 * W = Radstand
 * T = Breite
 * micha
 */
	#define differenzial_W 186
	#define differenzial_B 135
//Tachomter Berechnung
	#define MAGNETS_PER_WHEEL 48.0f
	#define WHEEL_RADIUS_METER 0.027f 	// radius of a wheel

// Drive Curve Parameters
	#define SQUARE_ANGLE_LEFT 			+60.0f
	#define SQUARE_ANGLE_RIGHT 	 		-60.0f
	#define SQUARE_VELOCITY			  	  1.0f
	#define SQUARE_DISTANCE_TO_STEER	  0.01f

#endif

#endif /* CONFIG_CAR_H_ */

