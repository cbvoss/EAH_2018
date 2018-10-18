/*
 * ted.h
 *
 *  Created on: 04.01.2017
 *      Author: Florian Schaedlich
 */

#ifndef TED_H_
#define TED_H_

void ted_initialize();

int ted_startbar_detection();

void ted_update();

enum track_event ted_get_track_event();

void ted_reset_track_event();

#endif /* TED_H_ */
