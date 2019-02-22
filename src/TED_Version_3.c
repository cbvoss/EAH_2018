/*
 * TED_Version_3.c
 *
 *  Created on: 15.01.2019
 *      Author: Maximilian Peuckert
 *      		Stefan Schmolke
 *
 *  Reviewed by:
 */

//includes
#include "TED_Version_3.h"
#include "distance_alarm.h"
#include "serial_blue.h"
#include "Ir_Board.h"
#include <stdio.h>
#include "config_car.h"
#include <math.h>

//defines
#define PICTURE_LENGTH 240
#define BOARD_LENGTH 8
#define RESOLUTION 0.004f

/*
 *  need to be changed according to tests
 */

#define DOUBLELINE_BRIGHT 1
#define DOUBLELINE_SHIFTS 1
#define RIGHTLINE_BRIGHT 1
#define RIGHTLINE_SHIFTS 1
#define LEFTLINE_BRIGHT 1
#define LEFTLINE_SHIFTS 1
#define NORM_BRIGHT 1
#define NORM_SHIFTS 1

#define LINETYPESIZE 4

char g_picture_binary[PICTURE_LENGTH];
char g_picture_binary_old[PICTURE_LENGTH];
char g_position_ringbuffer;

enum track_event g_current_event_TED3;

struct DistanceAlarm TED3_Distance;

/*
 * initialize variables to zero
 */
void Ted_Version_3_Initialize(){

	for(int i = 0; i < PICTURE_LENGTH - 1; i++)
		g_picture_binary[i] = 0;
	//g_position_ringbuffer = 0;

	distance_alarm_new_mean (&TED3_Distance, RESOLUTION, 1);


}
/*
 * send the picture over bluetooth
 */
void Ted_Picture_Print(){
	char buffer[256];
	for(int i = 0; i < PICTURE_LENGTH - 1; i++)	{
		sprintf(buffer, "%c\n", g_picture_binary);
		serial_blue_write_string(buffer);
	}
}
/*
 * reset the array values of the picture to 0
 */
void Ted_Picture_Reset(){
	for(int i=0; i < PICTURE_LENGTH; i++){
		g_picture_binary[i] = 0;
	}
}
/*
 * set the actual position of the array, based on the value from ir-board input
 * function call Ted_Increment_Ringbuffer
 */
void Ted_Picture_Create(char pattern){
	g_picture_binary[g_position_ringbuffer] = pattern;
	Ted_Increment_Ringbuffer();
}
/*
 * Increment the ringbuffer and rest them if position > 240 to 0
 */
int Ted_Increment_Ringbuffer(){
	int ringbuffer_reset;
	if(g_position_ringbuffer > PICTURE_LENGTH - 1){
		g_position_ringbuffer = 0;
		for(int i = 0; i < PICTURE_LENGTH; i++){
			g_picture_binary_old[i] = g_picture_binary[i];
		}
		ringbuffer_reset = 1;
	}
	else{
		g_position_ringbuffer ++;
		ringbuffer_reset = 0;
	}
	return ringbuffer_reset;
}
/*
 * TODO description of Ted_Picture_Update
 */
void Ted_Picture_Update(){
	int i = 7;
	char buff[64];
	for (int j = 0; j <= PICTURE_LENGTH-1; j++) {
			g_picture_binary[j] = ir_get_value(i);
			i -= 1;
			if (i<=-1)
				i = 7;
			sprintf(buff,"%c \n", g_picture_binary[j]);
			serial_blue_write_string(buff);

	}
	/*
	signed char buffer;
	for (int i=0; i < BOARD_LENGTH - 1; i++){
		buffer = ir_get_value(i);
		if (buffer == 1)
			if(i < 4)
				buffer = -1;
	}
	Ted_Picture_Create(buffer);
	*/
}
/*
 * calculated the brightness in the binary picture for the cluster analyze
 */
int Ted_Brightness_Calculation(){
	int one_identify = 0;
	int j = 0;
	for(int i=0; i < PICTURE_LENGTH - 1; i++){
		if(Ted_Popcount(g_picture_binary[i])) {
			if (j <= 3)
				one_identify -= 1;
			else
				one_identify++;
		}
		j++;
		if (j >= 7)
			j = 0;
	}
	return one_identify;
}
/*
 * calculated the line shifts in the binary picture for the cluster analyze
 */
int Ted_Line_Shift(){
	int shift = 0;
	int line_shift = 0;
	for (int i = 0; i <= PICTURE_LENGTH-9; i++) {
		shift = g_picture_binary[i] ^ g_picture_binary[i+8];
		if (shift)
			line_shift++;
	}
	return line_shift;


	/*
	char line_shift = 0;
	char line_new[BOARD_LENGTH];
	char line_old[BOARD_LENGTH];
	int n = 0;
	int buffer[BOARD_LENGTH];

	do{
		for(int i = 0; i < BOARD_LENGTH - 1; i++){
			line_old[i] = g_picture_binary[n * BOARD_LENGTH + i];
			line_new[i] = g_picture_binary[(n + 1) * BOARD_LENGTH + i];
			for(int j = 0; j < BOARD_LENGTH - 1; j++){
				buffer[i] = line_old[i] ^ line_new[i];
				if(Ted_Popcount(buffer[i]))
					line_shift++;
			}
		}
	}while(n < 30);
	return line_shift;
	*/
}
/*
 * simple popcount algorithm, detect 1
 */
int Ted_Popcount(char buffer){;
	if(buffer |= 0)
		return 1;
	else
		return 0;
}

/*
 * sends the brightness and shift values via bluetooth
 */

void ted_send(char brightness, char shifts) {
	char buffer[65];
	sprintf(buffer, "brightness %c shifts %c \n", brightness, shifts);
	serial_blue_write_string(buffer);

}

/*
 * calculates the euclidic distance of the brightness & shifts pair to
 * the predefined values
 */

char euclid(char brightness, char shifts) {
	int min;		//minimal Distance
	int minCount;	//Current Event which is the minimal
	float distance[4];

	/*
	 * Calculate the euclidic Distances to determine which Line should be detected
	 * sqrt((y2-y1)²+(x2-x1)²)
	 */
	distance[0] = sqrt((pow(DOUBLELINE_BRIGHT-brightness, 2)+pow(DOUBLELINE_SHIFTS-shifts, 2)));
	distance[1] = sqrt((pow(NORM_BRIGHT-brightness, 2)+pow(NORM_SHIFTS-shifts, 2)));
	distance[2] = sqrt((pow(RIGHTLINE_BRIGHT-brightness, 2)+pow(RIGHTLINE_SHIFTS-shifts, 2)));
	distance[3] = sqrt((pow(LEFTLINE_BRIGHT-brightness, 2)+pow(LEFTLINE_SHIFTS-shifts, 2)));

	min = distance[0];
	minCount = 0;
	for (int i = 1; i <= LINETYPESIZE -1; i++) {
		if (distance[i] < min) {
			min = distance[i];
			minCount = i;
		}
	}
	return minCount;
}

/*
 * Sets the detected Track event SLOW, NONE, JUMP_RIGHT, JUMP_LEFT
 */

void TED3_set_detected_track_event(int Type) {
	switch(Type) {
	case 0: g_current_event_TED3 = SQUARE;
			serial_blue_write_string("square");
			break;
	case 1: g_current_event_TED3 = NONE;
			break;
	case 2: g_current_event_TED3 = JUMP_RIGHT;
			serial_blue_write_string("right");
			break;
	case 3: g_current_event_TED3 = JUMP_LEFT;
			serial_blue_write_string("left");
			break;
	default: g_current_event_TED3 = NONE;
	}
}

/*
 * getter for the Track Event
 */

enum track_event TED3_get_track_event() {

	return g_current_event_TED3;
}
/*
 * gets called from the main function to update the track event
 */

void TED3_update() {
	char buff[64];
	//sprintf(buff, "1");
	//serial_blue_write_string(buff);
	if (distance_alarm_has_distance_reached(&TED3_Distance)){
		//sprintf(buff, "DRINNE\n");
		//serial_blue_write_string(buff);
		distance_alarm_reset(&TED3_Distance);
		//sprintf(buff, "RESET\n");
		//serial_blue_write_string(buff);
		int brightness;
		int shifts;
		Ted_Picture_Reset();
		//sprintf(buff, "PICRESET\n");
		//		serial_blue_write_string(buff);
		Ted_Picture_Update();
		//sprintf(buff, "UPDATE\n");
		//		serial_blue_write_string(buff);
		brightness = Ted_Brightness_Calculation();
		//sprintf(buff, "BRICALC\n");
		//				serial_blue_write_string(buff);
		shifts = Ted_Line_Shift();
		//sprintf(buff, "SHIFTS\n");
		//				serial_blue_write_string(buff);
		ted_send(brightness, shifts);
		//sprintf(buff, "SEND\n");
		//						serial_blue_write_string(buff);
		char type = euclid(brightness, shifts);
		//sprintf(buff, "EUCLID\n");
		//						serial_blue_write_string(buff);
		TED3_set_detected_track_event(type);

	}
}
/*
 * gets called from main function to reset the current track event
 */
void TED3_reset_track_event() {
	g_current_event_TED3 = NONE;
}
