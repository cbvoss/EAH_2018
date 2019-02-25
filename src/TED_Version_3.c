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
#define ROW_COUNT 30
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
char g_picture_buffer[PICTURE_LENGTH];
char g_position_ringbuffer;

enum track_event g_current_event_TED3;

struct DistanceAlarm g_TED3_Distance;

/*
 * initialize variables to zero
 */
void Ted_Version_3_Initialize(){

	for(int i = 0; i < PICTURE_LENGTH - 1; i++) {
		g_picture_binary[i] = 0;
		g_picture_buffer[i] = 0;
	}
	//g_position_ringbuffer = 0;

	distance_alarm_new_mean (&g_TED3_Distance, RESOLUTION, 1);


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
	/*
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
	*/
	return 0;
}


/*
 * puts 8 new values into the binary picture when called
 */
void Ted_Picture_Update(){
	//char buff[64];
	for (int i = 0; i <= BOARD_LENGTH-1; i++) {

		for (int j = 0; j <= PICTURE_LENGTH-1; j++) {
			g_picture_buffer[j] = g_picture_binary[j];
		}
		for (int k = 1; k <= PICTURE_LENGTH-1; k++) {
			g_picture_binary[k] = g_picture_buffer[k-1];
		}
		g_picture_binary[0] = ir_get_value(i);
	}
}

/*
 * calculated the brightness in the binary picture for the cluster analyze
 */
signed int Ted_Brightness_Calculation(){
	signed int one_identify = 0;
	int j = 0;
	for(int i=0; i <= PICTURE_LENGTH - 1; i++){
		if(g_picture_binary[i]) {
			if (j <= 3)
				one_identify--;
			else
				one_identify++;
		}
		j++;
		if (j > 7)
			j = 0;
	}

	return one_identify;
}
/*
 * calculated the line shifts in the binary picture for the cluster analyze
 */
char Ted_Line_Shift(){
	char shift = 0;
	char line_shift = 0;
	for (int i = 0; i < PICTURE_LENGTH-BOARD_LENGTH; i++) {
		shift = g_picture_binary[i] ^ g_picture_binary[i+BOARD_LENGTH];
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
 * sends the brightness and shift values via bluetooth
 */

void ted_send(signed int brightness, char shifts) {
	char buffer[65];
	sprintf(buffer, "brightness %d shifts %d \n", brightness, shifts);
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
			serial_blue_write_string("square \n");
			break;
	case 1: g_current_event_TED3 = NONE;
			break;
	case 2: g_current_event_TED3 = JUMP_RIGHT;
			serial_blue_write_string("right \n");
			break;
	case 3: g_current_event_TED3 = JUMP_LEFT;
			serial_blue_write_string("left \n");
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
int po = 0;
static int update_check = 0;
	if (distance_alarm_has_distance_reached(&g_TED3_Distance)){

		distance_alarm_reset(&g_TED3_Distance);
		signed int brightness;
		char shifts;
		Ted_Picture_Update();
/*		for (int i = 0; i <= 29; i++) {
			sprintf(buff, "|%d %d %d %d %d %d %d %d |\n", g_picture_binary[0+po], g_picture_binary[1+po], g_picture_binary[2+po], g_picture_binary[3+po], g_picture_binary[4+po], g_picture_binary[4+po], g_picture_binary[5+po], g_picture_binary[6+po], g_picture_binary[7+po]);
			serial_blue_write_string(buff);
			po+=8;
		}
*/
		sprintf(buff,"----------------------------------------- \n");
		serial_blue_write_string(buff);
		if (update_check >= ROW_COUNT-1) {
			brightness = Ted_Brightness_Calculation();
			shifts = Ted_Line_Shift();
			ted_send(brightness, shifts);
			char type = euclid(brightness, shifts);
			TED3_set_detected_track_event(type);
		}
		else update_check++;
	}
}
/*
 * gets called from main function to reset the current track event
 */
void TED3_reset_track_event() {
	g_current_event_TED3 = NONE;
}
