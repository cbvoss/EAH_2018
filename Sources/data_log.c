#include "config_car.h"
#ifdef DEBUG_data_log
#include "serial_blue.h"
#include "serial_com.h"
#include "drive.h"
#include "drive_curve.h"
#include "controller_hold_line.h"
#include "adc.h"
#include "servo.h"
#include <stdio.h>
#include "global_clock.h"
#include "tachometer.h"
#include "enums.h"

void data_log(struct timer *time,char stop){
	char str[256];
	//time;error;speed_r;speed_l;servo_value
	if(global_clock_timer(time) && !stop){
		global_clock_set_timer(time,4);
		snprintf(str, 256,"%.3f;%d;%d;%.3f\r\n",tachometer_get_extrapolated_arithemtic_mean_distance_meter(),drive_curve_get_memory_debug(),
		servo_get_position_pm(),drive_curve_get_I());
		serial_blue_write_string(str);
		serial_com_write_string(str);
	}
}
#endif
