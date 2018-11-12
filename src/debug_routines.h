/*
 * debug_routines.h
 *
 *  Created on: Jan 15, 2018
 *      Author: Tobias Hupel
 */

#ifndef DEBUG_ROUTINES_H_
#define DEBUG_ROUTINES_H_

void debug_drive_regulator(char startSign, int time1, int time2, int stoptime, int inTime,
		float initialV_mps, float secondV_mps, float thirdV_mps, char ubreak, float threshold);

void debug_ticks();

#endif /* DEBUG_ROUTINES_H_ */
