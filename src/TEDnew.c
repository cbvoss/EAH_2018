/*
 * TEDnew.c
 *
 *  Created on: 01.11.2017
 *      Author: Simon Michel
 *      Reviewed by: Tim Ole Steinbach
 */

#include "TEDnew.h"
#include "distance_alarm.h"
#include "serial_blue.h"
#include "Ir_Board.h"
#include <stdio.h>
#include "config_car.h"
#include "servo.h"
#include <math.h>

//#define DEBUG_ted_threshhold_median

#define BINARY_PICTURE_LENGTH 32
#define RESOLUTION 0.004f		//resolution of the picture (0.004f means a snap every 4mm)

//defines for naive line check
#define REGION_OF_INTEREST 2
#define BITS_OF_INTEREST 3

//defines that actually set the trackevent, in permille (500 means that something has been detected, that fits the given pattern 50%)
#define EVENT_SCORE_THRESH 800

//defines for the filter
#define FILTERKERN_LENGTH 64
#define FILTERKERN 0xFE000FFll		//the filter in hex numbers, must be long long

//defines for popcount (wikipedia)
#define m1 0x5555555555555555 //binary: 0101...
#define m2 0x3333333333333333 //binary: 00110011..
#define m4 0x0f0f0f0f0f0f0f0f //binary:  4 zeros,  4 ones ...

char g_binary_picture[BINARY_PICTURE_LENGTH];
char g_pointer_ringbuffer;
long g_Binary_Picture_Column[8];

char g_binary_picture_side_score[2];
int g_binary_picture_side_score_corr[2];
int g_binary_picture_side_score_median[2];

struct DistanceAlarm TED_Distance;
struct DistanceAlarm TED_waiting_dist;
enum track_event g_current_event_TEDnew;

int g_filter_normal;

char str[64];

char TED_active;

void TEDnew_activate()	{
	TED_active = 1;
}

void TEDnew_deactivate()	{
	TED_active = 0;
}

/**
 * calculates the hamming weight of a given 64bit (long long) number
 * @param x
 * 	the number to calculate the hamming weight for
 * @return
 * 	the hamming weight
 */
int TEDnew_popcount64bit(long long x)
{
    x -= (x >> 1) & m1;             //put count of each 2 bits into those 2 bits
    x = (x & m2) + ((x >> 2) & m2); //put count of each 4 bits into those 4 bits
    x = (x + (x >> 4)) & m4;        //put count of each 8 bits into those 8 bits
    x += x >>  8;  //put count of each 16 bits into their lowest 8 bits
    x += x >> 16;  //put count of each 32 bits into their lowest 8 bits
    x += x >> 32;  //put count of each 64 bits into their lowest 8 bits
    return x & 0x7f;
}

/**
 * Prints the full binary picture over bluetooth
 */
void TED_print_picture_blue()	{
	int i;
	for(i = 0; i<BINARY_PICTURE_LENGTH; i++)	{
		snprintf(str, 256, "%c\n", g_binary_picture[i]);
		serial_blue_write_string(str);
	}
}
/**
 * fully clears everything related to the binary picture, used after track event
 */
void TEDnew_reset_picture()	{
	for (int i= 0 ; i<8; i++)	g_Binary_Picture_Column[i] = 0;
	for(int i = 0; i< BINARY_PICTURE_LENGTH; i++)	g_binary_picture[i] = 0;

	g_current_event_TEDnew = NONE;
}

/**
 * getter for the score for every side (score in permille, 0 meaning that there has not been detected a pattern on the given side at all, 1000 meaning that there was 100% without a doubt a pattern on that side
 * @param side
 * 	side of interest, left or right
 * @return
 * 	the score on the side
 */
int TEDnew_get_score(enum Side side)  {
    return g_binary_picture_side_score_corr[side];
}

/**
 * since the picture is in a ringbuffer, this function keeps track of the pointer, that points on the start of that ringbuffer
 * @return
 * 	dummy
 */
char TEDnew_increment_pointer_ringbuffer()	{
	if(g_pointer_ringbuffer >= BINARY_PICTURE_LENGTH - 1)	{
		g_pointer_ringbuffer = 0;
#ifdef DEBUG_binary_picture
		TED_print_picture_blue();
#endif
	}
	else	g_pointer_ringbuffer++;
	return 0;
}

/**
 * getter for the actual track event, that has been detected
 * @return
 * 	the track event, that TEDnew decided for
 */
enum track_event TEDnew_get_track_event()
{
    return g_current_event_TEDnew;
}

/**
 * Sets the actual track event, based on the scores in permille for each side and a defined threshold, also in permille (a threshold of 800 means, that the event is set if the algorithm is 80% certain there is a pattern on that side)
 * @param g_binary_picture_side_score
 * 	score to evaluate
 * @param event_score_threshold
 * 	the actual threshold
 */
void TEDnew_set_current_track_event(int *binary_picture_side_score, int event_score_threshold)	{
	if(servo_get_position_angle_deg() <= 10)	{
	if(binary_picture_side_score[LEFT] >= event_score_threshold){
		if (distance_alarm_has_distance_reached (&TED_waiting_dist))	{
			g_current_event_TEDnew = JUMP_LEFT;
			distance_alarm_reset(&TED_waiting_dist);
		}
	}
	else if(binary_picture_side_score[RIGHT] >= event_score_threshold) g_current_event_TEDnew = JUMP_RIGHT;
	}
	else	g_current_event_TEDnew = NONE;

	if((binary_picture_side_score[LEFT] >= event_score_threshold) && (binary_picture_side_score[RIGHT]>= event_score_threshold) && (servo_get_position_angle_deg() <= 10))	{
	//	if (distance_alarm_has_distance_reached (&TED_waiting_dist))	{
			g_current_event_TEDnew = SQUARE;
	//		distance_alarm_reset(&TED_waiting_dist);
	//	}
	}
}

//---------------------------------------Naive Line check-------------------------------

int TEDnew_check_4_white(long *Binary_Picture_Column, char pointer, char column) {
    char zaehler_eins = 0;
    char i;

    for(i = pointer; i < BINARY_PICTURE_LENGTH; i++)	{

        if ((Binary_Picture_Column[column]&(1<<i)) != 0)    zaehler_eins++;
        if (zaehler_eins == BITS_OF_INTEREST-1) {
            return i;
        }
        if ((Binary_Picture_Column[column]&(1<<i)) == 0) zaehler_eins = 0;
    }
    return 0;
}

char TEDnew_decide_for_starting_column(enum Side side) {
    if(side == RIGHT)    return 0;
    if(side == LEFT)   return 4;
    return 0;
}

void Binary_Picture_Check_SQUARE(long *Binary_Picture_Column) {
    enum Side side;

    char i,j;
    char pointer = 0;

	for (side = RIGHT; side <= LEFT; side++) ;

    for (side = RIGHT; side <= LEFT; side++)   {
        char column_to_start = TEDnew_decide_for_starting_column(side);

        for(i=column_to_start; i <= column_to_start+3; i++) {

            j = TEDnew_check_4_white(Binary_Picture_Column, pointer, i);

            /*snprintf(str, 256, "%u  ; %u\n",j , j);
            serial_blue_write_string(str); */

            if(j != 0) {
                //increment_score(side);
                if(j >= (REGION_OF_INTEREST + BITS_OF_INTEREST))    pointer = (j - REGION_OF_INTEREST - BITS_OF_INTEREST);
                else pointer = 0;
            }
        }
    }
}


//----------------------------Line check with correlation-------------------

/**
 * checks a given column of the binary picture for the pattern, based on correlation:
 * e.g: if this is the picture
 *
 * 00011000
 * 11111111
 * 00011000
 * 00110000
 * 00011000
 * 00001100
 * 11111111
 * 00011000
 *
 * this function will correlate one column of that picture (or any other column) with a mask an will return the hamming weight of that column
 *
 * @param filter
 * 	the mask to cehck the column against
 * @param Binary_Picture_Column
 * 	pointer to the arry of the columns (which are just 8 numbers tbh)
 * @param column
 * 	# of the column we are looking at
 * @return
 * 	the correlated score of the side, NOT in permille
 */
int TEDnew_correlate_4_white(long long filter, long *Binary_Picture_Column, char column)    {
    int i, highest_corr_score = 0, z;
    long long j = 0;
    for(i = FILTERKERN_LENGTH; i>= -FILTERKERN_LENGTH; i--)    {
        if(i>=0)    j = (filter<<i) & (long long)Binary_Picture_Column[column];
        else        j = (filter>>(-i)) & (long long)Binary_Picture_Column[column];
        	z = TEDnew_popcount64bit(j);
        if(z > highest_corr_score)   highest_corr_score = z;
    }
    return highest_corr_score;
}

/**
 * This is where the actual checking for the pattern happens. every column is checked and according to the side it is on, added to the score of that side, while turning the score in a permille
 * @param Binary_Picture_Column
 * 	the array of the columns
 * @return
 * 	dummy
 */
char TEDnew_Binary_Picture_Check_Corr(long *Binary_Picture_Column) {
    enum Side side;
    int i,j, counter[2];
    float side_score_total=0;

    for (side = RIGHT; side <= LEFT; side++)   {
    	counter[side]++;
        char column_to_start = TEDnew_decide_for_starting_column(side);

        for(i=column_to_start; i<=column_to_start+3; i++) {

            j = TEDnew_correlate_4_white(FILTERKERN, Binary_Picture_Column, i);
            side_score_total += j;
        }
        	g_binary_picture_side_score_corr[side] = (int)(1000 * side_score_total/(g_filter_normal * 4));		//for permille
        	side_score_total = 0;
    }
    return 0;
}

/**
 * since the picture is in a ringbuffer we acutally would need the pointer on the start everytime we look a t it. but since we are messing with the raw picture anyway by turning it into columns we can also adjust the columns in a way, that they always start with the start of the picture, which is done in this function
 * @param i
 * 	???
 * @param pointer
 * 	pointer on the ringbuffer
 * @return
 */
int TEDnew_get_rid_of_that_pointer(int i, char pointer)	{
	if(i + pointer < BINARY_PICTURE_LENGTH)	return (i + pointer);
	else return (pointer + i - BINARY_PICTURE_LENGTH);
}

/**
 * turns the picture in a columns, or strictly speaking an array of 8 numbers that represent the columns in binary
 * @param Binary_Picture
 * 	the array of the picture
 * @param g_Binary_Picture_Column
 * 	the array of the columns
 */
void Binary_Picture_to_Colum (char *Binary_Picture, long *g_Binary_Picture_Column){

    int i, j, k;

    for(i = 0; i<8; i++)	g_Binary_Picture_Column[i] = 0;

    for (i=0; i<BINARY_PICTURE_LENGTH; i++){
        for (j=0 ; j<8; j++){
        	k = TEDnew_get_rid_of_that_pointer(i, g_pointer_ringbuffer);
            if((Binary_Picture[k]&(1<<j)) != 0) {
                g_Binary_Picture_Column[j] = g_Binary_Picture_Column[j] | (1 << i);
            }
        }
    }
}


void calculate_median_score(int *score)	{
	enum Side side;
	static int scores[2][4];
	static int counter = 0;
	static float score_median[2];

	for (side = RIGHT; side <= LEFT; side++) scores[side][counter] = score[side];

		if(counter == 3)	{
			counter = 0;

			for(int i = 0; i <= 3; i++)	{
				for (side = RIGHT; side <= LEFT; side++) 	score_median[side] += scores[side][i];
			}

			for (side = RIGHT; side <= LEFT; side++)	{
				(int)g_binary_picture_side_score_median[side] = score_median[side]/4.0f;
				score_median[side] = 0;
			}

#ifdef DEBUG_ted_threshhold
 	       snprintf(str, 64, "Median Score: %u  : %u\n", g_binary_picture_side_score_median[LEFT], g_binary_picture_side_score_median[RIGHT]);
 	       serial_blue_write_string(str);
#endif

		}
		else counter++;
}

/**
 * init
 */
void TEDnew_picture_initialize()
{
	enum Side side;
	g_pointer_ringbuffer = 0;
	distance_alarm_new_mean (&TED_Distance, RESOLUTION, 1);
	distance_alarm_new_mean(&TED_waiting_dist, 0.02f, 1);
	g_current_event_TEDnew = NONE;

	g_filter_normal = TEDnew_popcount64bit(FILTERKERN);

    for (int i= 0 ; i<8; i++)	g_Binary_Picture_Column[i] = 0;
	for(int i = 0; i< BINARY_PICTURE_LENGTH; i++)	g_binary_picture[i] = 0;
	
	for (side = RIGHT; side <= LEFT; side++) {
	g_binary_picture_side_score_corr[side] = 0;

	TED_active = 1;

	}
}

/**
 * "snaps" the state of the sensor boards and puts it in a ringbuffer
 * @param current_row
 * 	state of the sensor board
 * @return
 * 	dummy
 */
char TEDnew_build_binary_picture(char current_row)	{

        g_binary_picture[g_pointer_ringbuffer] = current_row;
        TEDnew_increment_pointer_ringbuffer();

	return 0;
}

/**
 * updater, the order of the functions should not be changed
 */
void TEDnew_Update()	{

	   if (distance_alarm_has_distance_reached (&TED_Distance))	{
		   if(fabsf(servo_get_position_angle_deg()) <= 10 && TED_active != 0)	{

	       distance_alarm_reset (&TED_Distance);
	       TEDnew_build_binary_picture(ir_get_pattern());
	       Binary_Picture_to_Colum(g_binary_picture, g_Binary_Picture_Column);
	       TEDnew_Binary_Picture_Check_Corr(g_Binary_Picture_Column);
	       calculate_median_score(g_binary_picture_side_score_corr);

#ifndef DEBUG_ted_threshhold
	       TEDnew_set_current_track_event(g_binary_picture_side_score_median, EVENT_SCORE_THRESH);
#endif
		   }
#ifdef DEBUG_ted_threshhold
 	       snprintf(str, 64, "%u  : %u\n", g_binary_picture_side_score_corr[LEFT], g_binary_picture_side_score_corr[RIGHT]);
 	       serial_blue_write_string(str);
#endif
	  }
}
