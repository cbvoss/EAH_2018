/**
* This is a mock up for the real hall module.
**/

#ifndef HALL_H_
#define HALL_H_

#include "../../src/enums.h"
#include "../../src/global_clock.h"

unsigned int hall_get_current_counts_per_tick(enum wheel_selector wheel);
unsigned int hall_get_previous_counts_per_tick(enum wheel_selector wheel);
unsigned int hall_get_counter_prescaler();
unsigned int hall_get_tick_count(enum wheel_selector wheel);
void hall_initialize(void);

void hall_setHallTickLeft();
void hall_setHallTickRight();
#endif // HALL_H_
