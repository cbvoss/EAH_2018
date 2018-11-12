/*
 * controller_hold_line.c
 *
 *  Created on: 21.12.2016
 *      Author: franz luebke
 *      Review:
 *      Franz Lübke
 *      Adrian Zentgraf
 */
#include "controller_hold_line.h"
#include "adc.h"
#include "regulator.h"
#include "drive.h"
#include "enums.h"
#include "typedefine.h"
#include "tachometer.h"
#include "fuzzy_sniffer.h"
#include "Ir_Board.h"
#include "servo.h"
#include "global_clock.h"
#include "iodefine.h"
#include "drive_curve.h"
#include "config_car.h"

#define HOLD_CURVE

#define IR_ANALOG_ZERO_DRIFT 127.0f

char hold_line_good_position(char Epsilon){

	return drive_curve_good_position();
	/*
	_UBYTE left_ir = adc_get_value(LEFT_IR);
	_UBYTE right_ir = adc_get_value(RIGHT_IR);
	return ((((left_ir<(GRAY+Epsilon))) && ((GRAY-Epsilon)<left_ir)) && (((right_ir<(GRAY+Epsilon))) && ((GRAY-Epsilon)<right_ir)));
	*/
}

#ifdef HOLD_CURVE

void hold_line_initalize(void){}
void hold_line_fixed_update(void){}
void hold_line_update(void){}
void hold_line_set_max_speed(float speed){
	drive_curve_set_max_speed(speed);
}
void hold_line_reset(){
	drive_curve_reset();
}
void hold_line_set_active(char active){
	drive_curve_set_active(active);
}
#endif

#ifdef HOLD_LINE


#define FIXED_WHEEL 0
#define HOLD_LINE_USE_BREAK 0
#define REGULATOR_V 0.002f
#define SNIFF_V 10
#define SERVO_DAMPING 1.0f
#define IR_EPSILON 20

/*
 * Anpassung des servo sniffv
 *
 */




#define CURVE_EPSILON 20
#define SMALL_CURVE 400
#define BIG_CURVE 600
#define SMALL_CURVE_OFFSET 400
#define BIG_CURVE_OFFSET 600



struct Regulator g_line_regulator;
struct Regulator g_line_servo_regulator;
struct FuzzySniffer g_line_sniff;
struct timer g_hold_line_timer;
struct timer g_hold_line_timer1;
float g_hold_line_max_speed=0.0f;
float g_hold_line_end_speed=0.0f;
int g_hold_line_speed_delay=2;
volatile float g_hold_line_regval=0;
char g_hold_line = 0;


/**
 * @note
 * returns a value proportional to the drift
 * @return float
 * value proportional to the drift
 */
float hold_line_get_error(void){
	return (IR_ANALOG_ZERO_DRIFT - (float)adc_get_value(SUB_RESULT)) ;
}

/**
 * @note
 * Initialize the hold line module
 * 	-PID values
 * 	-initial max speed
 */
void hold_line_initalize(){
	regulator_new(&g_line_regulator,REGULATOR_V ,0.0002f,0.0003f,0.001f);
	regulator_new(&g_line_servo_regulator,16.0f,0.0f,0.0004f,0.001f);
	g_hold_line_max_speed=1.5f;
	fuzzy_sniffer_new(&g_line_sniff,0x18,SNIFF_V,3);
	global_clock_set_timer(&g_hold_line_timer,1);
	global_clock_set_timer(&g_hold_line_timer1,1);
	fuzzy_sniffer_new(&g_line_sniff,0x18,SNIFF_V,3);
	PORTD.DDR.BIT.B0=1;
	PORTD.DR.BIT.B0=1;
}

void hold_line_set_active(char active){
	g_hold_line = (active)?1:0;

}

void hold_line_curve(int radius, enum Side side){
	switch(side){
	case RIGHT:
		if(((SMALL_CURVE-CURVE_EPSILON)<radius) && (radius<(SMALL_CURVE+CURVE_EPSILON))){
			//alter to curve pattern
			fuzzy_sniffer_new(&g_line_sniff,0xc0,SNIFF_V,3);
			servo_set_position_pm(-1*SMALL_CURVE_OFFSET);
		}
		if(BIG_CURVE-CURVE_EPSILON<radius<BIG_CURVE+CURVE_EPSILON){
			fuzzy_sniffer_new(&g_line_sniff,0xc0,SNIFF_V,3);
			servo_set_position_pm(SMALL_CURVE_OFFSET);
		}
		break;
	case LEFT:
		if(SMALL_CURVE-CURVE_EPSILON<radius<SMALL_CURVE+CURVE_EPSILON){
			//alter to curve pattern
			fuzzy_sniffer_new(&g_line_sniff,0x3,SNIFF_V,3);
			servo_set_position_pm(-1*BIG_CURVE_OFFSET);
		}
		if(BIG_CURVE-CURVE_EPSILON<radius<BIG_CURVE+CURVE_EPSILON){
			fuzzy_sniffer_new(&g_line_sniff,0x3,SNIFF_V,3);
			servo_set_position_pm(BIG_CURVE_OFFSET);

		}
		break;
	}
}

float hold_line_aretmetig_middle_velocity(){
	return (tachometer_get_velocity_mps(BACK_RIGHT) + tachometer_get_velocity_mps(BACK_LEFT)) / 2.0f;
}

float hold_line_norm_regulator_value(float regval){

	float result = regval  * hold_line_aretmetig_middle_velocity();
	return result;
}


/**
 *
 */
void hold_line_with_servo(){
		_UBYTE left_ir = adc_get_value(LEFT_IR);
		_UBYTE right_ir = adc_get_value(RIGHT_IR);
		if(!hold_line_good_position(IR_EPSILON)){
			servo_set_position_pm(hold_line_get_servo_regulator_value());
		}
}


/**
 * @note
 * must be updated equidistant
 *
 * starts the acd convertion
 * calculate the regulator
 */
void hold_line_fixed_update(){
	if(g_hold_line){
		hold_line_with_servo();
		g_hold_line_regval=regulator_calculate_value(&g_line_regulator,hold_line_get_error());
	}
}

/**
 *
 * @param speed set the max speed to use
 */
void hold_line_set_max_speed(float speed){
	g_hold_line_max_speed=speed;
}

/**
 * @note
 * steer the car in the center of the path
 * @param reg_value
 * value calculated from a regulator to minimize the drift
 */
void hold_line_controller_switching_wheel(float reg_value){
	float rval = 0.0f;
	if(reg_value){
		if(reg_value>0){//right
			rval=((g_hold_line_max_speed-reg_value)<0.0f)?0.0f:g_hold_line_max_speed-reg_value;
			drive_accelerate_to(LEFT,g_hold_line_max_speed,HOLD_LINE_USE_BREAK);
			drive_accelerate_to(RIGHT,rval,1);
		}else{//left
			rval=((g_hold_line_max_speed+reg_value)<0.0f)?0.0f:g_hold_line_max_speed+reg_value;
			drive_accelerate_to(LEFT,rval,1);
			drive_accelerate_to(RIGHT,g_hold_line_max_speed,HOLD_LINE_USE_BREAK);
		}
	}else{
		drive_accelerate_to(LEFT,g_hold_line_max_speed,HOLD_LINE_USE_BREAK);
		drive_accelerate_to(RIGHT,g_hold_line_max_speed,HOLD_LINE_USE_BREAK);
	}
}

int hold_line_get_servo_regulator_value(){
	float regval = regulator_calculate_value(&g_line_servo_regulator,(float)fuzzy_sniffer_calculate_servo_delta(&g_line_sniff, ir_get_pattern()));
	return (int)(regval/(hold_line_aretmetig_middle_velocity()+1.0f));
}


void hold_line_speed_ramp(float end_speed){
	g_hold_line_end_speed=end_speed;
}


void hold_line_controller_fixed_wheel(float reg_value){
	float regval = (g_hold_line_max_speed+reg_value);
	regval=hold_line_norm_regulator_value(reg_value);
	if(regval<0.0f) regval=0.0f;
	drive_accelerate_to(RIGHT,g_hold_line_max_speed,HOLD_LINE_USE_BREAK);
	drive_accelerate_to(LEFT,regval , HOLD_LINE_USE_BREAK);
}

/**
 * @note
 * this function must be used continuously to activate the hold_line_controller_switching_wheel
 */
void hold_line_update(){
	if(g_hold_line){
#if FIXED_WHEEL
		hold_line_controller_fixed_wheel(hold_line_norm_regulator_value(g_hold_line_regval));
#else
		hold_line_controller_switching_wheel(hold_line_norm_regulator_value(g_hold_line_regval));
#endif
		global_clock_set_timer(&g_hold_line_timer1,g_hold_line_speed_delay);
		if(global_clock_timer(&g_hold_line_timer1) && g_hold_line_end_speed>g_hold_line_max_speed){
			global_clock_set_timer(&g_hold_line_timer1,1);
			g_hold_line_max_speed+=0.1f;
		}
	}
}

float hold_line_get_regulator_value(){
	return hold_line_norm_regulator_value(g_hold_line_regval);
}
#endif
