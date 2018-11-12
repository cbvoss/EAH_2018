/*
 * TEDnew.h
 *
 *  Created on: 01.11.2017
 *      Author: Simon
 */

#ifndef TEDNEW_H_
#define TEDNEW_H_

char get_score(enum Side side);

void TEDnew_picture_initialize();

void TEDnew_Update();

void TEDnew_reset_picture();

int TEDnew_get_score(side);

enum track_event TEDnew_get_track_event();

void TEDnew_activate();

void TEDnew_deactivate();

#endif /* TEDNEW_H_ */
