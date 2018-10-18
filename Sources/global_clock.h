/*
 * global_clock.h
 *
 *  Created on: 03.01.2017
 *      Author: franz_lübke
 */

#ifndef GLOBAL_CLOCK_H_
#define GLOBAL_CLOCK_H_

struct timer{
	unsigned int init_count;
	unsigned int timer_value;
};

void global_clock_fixed_update();
unsigned int global_clock_get_count();
char global_clock_timer(struct timer *time);
void global_clock_set_timer(struct timer *time, unsigned int countdown);
void global_clock_reset_timer(struct timer *time);
void global_clock_wait(unsigned int ms);
void global_clock_set_100us_timer(struct timer *time, unsigned int countdown);
unsigned int global_clock_get_100us_count();

#endif /* GLOBAL_CLOCK_H_ */
