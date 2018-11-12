/*
 * global_clock.c
 *
 *  Created on: 03.01.2017
 *      Author: franz_lübke
 *      Reviewed by:
 *
 *      Adrian
 *      Franz
 */
#include "global_clock.h"
volatile unsigned int g_global_clock = 0;

void global_clock_fixed_update() {
	g_global_clock++;
}

/**
 * vergangene zeit in ms seit start
 * @return vergangene zeit in 1ms
 */
unsigned int global_clock_get_count() {
	return g_global_clock/10;
}
/**
 * vergangene zeit in 100us seit start
 * @return vergangene zeit in 100us zählungen
 */
unsigned int global_clock_get_100us_count() {
	return g_global_clock;
}
/**
 * prüft ob der timer abgelaufen ist
 * @param time ->pointer auf timer struckt welches geprüft werden soll
 * @return 1/true wenn der timer abgelaufen ist 0/false wenn nicht
 */
char global_clock_timer(struct timer *time) {
	return ((global_clock_get_100us_count() - time->init_count) >= time->timer_value) ? 1 : 0;
}
/**
 * initialisiert einen timer
 * @param time -> pointer auf timer struct
 * @param countdown -> wert in ms der "ablaufen soll"
 */
void global_clock_set_timer(struct timer *time, unsigned int countdown) {
	time->init_count = global_clock_get_100us_count();
	time->timer_value = 10*countdown;
}
/**
 * initialisiert einen timer
 * @param time -> pointer auf timer struct
 * @param countdown -> wert in 100us der "ablaufen soll"
 */
void global_clock_set_100us_timer(struct timer *time, unsigned int countdown) {
	time->init_count = global_clock_get_100us_count();
	time->timer_value = countdown;
}

/**
 * Resets the timer
 *
 * @param time
 * 		the timer
 */
void global_clock_reset_timer(struct timer *time)
{
	time->init_count = global_clock_get_100us_count();
}

/**
 * Waits the given number of milliseconds.
 * @param tickCount
 * 	The time to wait in milliseconds.
 */
void global_clock_wait(unsigned int ms){
	volatile unsigned int targetTick = ms + global_clock_get_count();

	while (global_clock_get_count() < targetTick){
	};
}
