/*
 * TED_Version_3.c
 *
 *  Created on: 15.01.2019
 *      Author: Maximilian Peuckert
 *
 *  Reviewed by:
 */

//includes
#include "TED_Version_3.h"

//defines
#define PICTURE_LENGTH 240
#define BOARD_LENGTH 8

char g_picture_binary[PICTURE_LENGTH];
char g_picture_binary_old[PICTURE_LENGTH];
char g_position_ringbuffer;


/*
 * initialize variables to zero
 */
void Ted_Version_3_Initialize(){
	for(int i = 0; i < PICTURE_LENGTH - 1; i++)
		g_picture_binary[i] = 0;
	g_position_ringbuffer = 0;

}
/*
 * send the picture over bluetooth
 */
void Ted_Picture_Print(){
	char buffer[64];
	for(int i = 0; i < PICTURE_LENGTH - 1; i++)	{
		snprintf(buffer, 256, "%c\n", g_picture_binary);
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
	signed char buffer;
	for (int i=0; i < BOARD_LENGTH - 1; i++){
		buffer = ir_get_value(i);
		if (buffer == 1)
			if(i < 4)
				buffer = -1;
	}
	Ted_Picture_Create(buffer);
}
/*
 * calculated the brightness in the binary picture for the cluster analyze
 */
int Ted_Brightness_Calulation(){
	int one_identify = 0;
	for(int i=0; i < PICTURE_LENGTH - 1; i++){
		if(Ted_Popcount(g_picture_binary[i]) == 1)
			one_identify++;
	}
	return one_identify;
}
/*
 * calculated the line shifts in the binary picture for the cluster analyze
 */
char Ted_Line_Shift(){
	char line_shift = 0;
	char line_new[BOARD_LENGTH];
	char line_old[BOARD_LENGTH];
	int n = 0;
	int buffer;

	do{
		for(int i = 0; i < BOARD_LENGTH - 1; i++){
			line_old[i] = g_picture_binary[n * BOARD_LENGTH + i];
			line_new[i] = g_picture_binary[(n + 1) * BOARD_LENGTH + i];
			for(int j = 0; j < BOARD_LENGTH - 1; j++){
				buffer[i] = line_old[i] ^ line_new[i];
				if(Ted_Popcount(buffer[i]) == 1)
					line_shift++;
			}
		}
	}while(n < 30);
	return line_shift;
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

