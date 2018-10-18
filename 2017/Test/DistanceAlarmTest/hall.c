#include "hall.h"

#define COUNTER_FREQUENCY_HZ 50000000
#define COUNTER_PRESCALER 1024			// Prescaler of counters
#define COUNTER_PERIOD_TIME_S ((float)COUNTER_PRESCALER/COUNTER_FREQUENCY_HZ)

struct wheels {
	struct wheel {
		unsigned int tick_count;
		unsigned int ovf_count;
		unsigned short current_counts_per_tick;
		unsigned short previous_counts_per_tick;
	} FRONT_LEFT, FRONT_RIGTH, BACK_LEFT, BACK_RIGHT;
};
volatile struct wheels g_hall_wheel;

unsigned int hall_get_counter_prescaler()
{
	return COUNTER_PRESCALER;
}

unsigned int hall_get_tick_count(enum wheel_selector wheel) {
	switch (wheel) {
	case FRONT_LEFT:
		return g_hall_wheel.FRONT_LEFT.tick_count;
		break;
	case FRONT_RIGHT:
		return g_hall_wheel.FRONT_RIGTH.tick_count;
		break;
	case BACK_LEFT:
		return g_hall_wheel.BACK_LEFT.tick_count;
		break;
	case BACK_RIGHT:
		return g_hall_wheel.BACK_RIGHT.tick_count;
		break;
	default:
		return 0;
	}
	return 0;
}

unsigned int hall_get_current_counts_per_tick(enum wheel_selector wheel) {
	switch (wheel) {
	case FRONT_LEFT:
		return g_hall_wheel.FRONT_LEFT.current_counts_per_tick;
		break;
	case FRONT_RIGHT:
		return g_hall_wheel.FRONT_RIGTH.current_counts_per_tick;
		break;
	case BACK_LEFT:
		return g_hall_wheel.BACK_LEFT.current_counts_per_tick;
		break;
	case BACK_RIGHT:
		return g_hall_wheel.BACK_RIGHT.current_counts_per_tick;
		break;
	default:
		return 0;
	}
	return 0;
}

unsigned int hall_get_previous_counts_per_tick(enum wheel_selector wheel) {
	switch (wheel) {
	case FRONT_LEFT:
		return g_hall_wheel.FRONT_LEFT.previous_counts_per_tick;
		break;
	case FRONT_RIGHT:
		return g_hall_wheel.FRONT_RIGTH.previous_counts_per_tick;
		break;
	case BACK_LEFT:
		return g_hall_wheel.BACK_LEFT.previous_counts_per_tick;
		break;
	case BACK_RIGHT:
		return g_hall_wheel.BACK_RIGHT.previous_counts_per_tick;
		break;
	default:
		return 0;
	}
	return 0;
}

/**
* former "void Excep_MTU3_TGIC3(void)".
* Simulates a hall tick based on the global clock.
**/
void hall_setHallTickLeft()
{
    static int lastTick = 0;
    int currentTick;
    float deltaTick;
    currentTick = global_clock_get_100us_count();
    deltaTick = (float)(currentTick - lastTick) / 10000 / COUNTER_PERIOD_TIME_S;
    lastTick = currentTick;

	g_hall_wheel.BACK_LEFT.previous_counts_per_tick=g_hall_wheel.BACK_LEFT.current_counts_per_tick;
	//g_hall_wheel.BACK_LEFT.current_counts_per_tick = (g_hall_wheel.BACK_LEFT.ovf_count << 16) + MTU3.TGRC;
	g_hall_wheel.BACK_LEFT.current_counts_per_tick = (int)deltaTick;
	g_hall_wheel.BACK_LEFT.tick_count++;
	g_hall_wheel.BACK_LEFT.ovf_count = 0;
	//MTU3.TSR.BIT.TGFC = 0;
}

/**
* former "void Excep_MTU3_TGIC3(void)".
* Simulates a hall tick based on the global clock.
**/
void hall_setHallTickRight()
{
    static int lastTick = 0;
    int currentTick;
    float deltaTick;
    currentTick = global_clock_get_100us_count();
    deltaTick = (float)(currentTick - lastTick) / 10000 / COUNTER_PERIOD_TIME_S;
    lastTick = currentTick;

	g_hall_wheel.BACK_RIGHT.previous_counts_per_tick=g_hall_wheel.BACK_RIGHT.current_counts_per_tick;
	//g_hall_wheel.BACK_RIGHT.current_counts_per_tick = (g_hall_wheel.BACK_RIGHT.ovf_count << 16) + MTU6.TGRB;
	g_hall_wheel.BACK_RIGHT.current_counts_per_tick = (int)deltaTick;
	g_hall_wheel.BACK_RIGHT.tick_count++;
	g_hall_wheel.BACK_RIGHT.ovf_count = 0;
	//MTU6.TSR.BIT.TGFB = 0;
}

void hall_initialize()
{
    g_hall_wheel.BACK_LEFT.tick_count=0;
	g_hall_wheel.BACK_RIGHT.tick_count=0;
	g_hall_wheel.FRONT_LEFT.tick_count=0;
	g_hall_wheel.FRONT_RIGTH.tick_count=0;

	g_hall_wheel.BACK_LEFT.ovf_count=0xffff;
	g_hall_wheel.BACK_LEFT.current_counts_per_tick=0xffff;
	g_hall_wheel.BACK_LEFT.previous_counts_per_tick=0xffff;

	g_hall_wheel.BACK_RIGHT.ovf_count=0xffff;
	g_hall_wheel.BACK_RIGHT.current_counts_per_tick=0xffff;
	g_hall_wheel.BACK_RIGHT.previous_counts_per_tick=0xffff;

	g_hall_wheel.FRONT_LEFT.ovf_count=0xffff;
	g_hall_wheel.FRONT_LEFT.current_counts_per_tick=0xffff;
	g_hall_wheel.FRONT_LEFT.previous_counts_per_tick=0xffff;

	g_hall_wheel.FRONT_RIGTH.ovf_count=0xffff;
	g_hall_wheel.FRONT_RIGTH.current_counts_per_tick=0xffff;
	g_hall_wheel.FRONT_RIGTH.previous_counts_per_tick=0xffff;
}
