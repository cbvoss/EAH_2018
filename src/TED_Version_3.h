/*
 * TED_Version_3.h
 *
 *  Created on: 15.01.2019
 *      Author: Maximilian Peuckert
 */

#include "distance_alarm.h"
#include "serial_blue.h"
#include "Ir_Board.h"
#include <stdio.h>
#include "config_car.h"
#include "servo.h"
#include <math.h>

void Ted_Version_3_Initialize();
void Ted_Picture_Print();
void Ted_Picture_Reset();
void Ted_Picture_Create(char pattern);
int Ted_Increment_Ringbuffer();
void Ted_Picture_Update();
int Ted_Brightness_Calulation();
char Ted_Line_Shift();
int Ted_Popcount();
