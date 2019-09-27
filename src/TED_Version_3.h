/*
 * TED_Version_3.h
 *
 *  Created on: 15.01.2019
 *      Author: Maximilian Peuckert
 */

#ifndef TED3_H_
#define TED3_H_

void Ted_Version_3_Initialize();
void Ted_Picture_Print();
void Ted_Picture_Reset();
void Ted_Picture_Create(char pattern);
int Ted_Increment_Ringbuffer();
void Ted_Picture_Update();
signed int Ted_Brightness_Calculation();
char Ted_Line_Shift();
int Ted_Popcount(char buffer);
void ted_send(signed int brightness, char shifts);
char euclid(char brightness, char shifts);
void TED3_set_detected_track_event(int Type);
enum track_event TED3_get_track_event();
void TED3_update();
void TED3_reset_track_event();

#endif /* TED3_H_ */
