/*
 * opto_interruptor.c
 *
 *	Implementation of Driver to use Opto sensors
 *
 *  Created on: 12.11.2016
 *      Author: Franz Lübke
 *      		Adrian Zentgraf
 *
 *      Reviewed on 12.02.2016 by:
 *      		Franz Lübke
 *      		Eric Elsner
 *      		Tobias Hupel
 *
 *      Modified on 15.01.2019 by:
 *     			Heiko Gericke
 */
#include "iodefine.h"
#include "opto_interruptor.h"
#include "vect.h"

//TEST 123

#define COUNTER_PRESCALER 1024			// Prescaler of counters
#define WHEEL_COUNT 4

#define RINGBUFFER_INDEX(LENGTH, OFFSET) ((OFFSET) % (LENGTH))


//--------------------------------------
void debug_ticks();
//--------------------------------------



/**
   Global variables Opto
 * sum Counts of wheel ticks
 */
struct OptoWheel
{
	uint32_t tick_count;
	uint32_t ovf_count;
	uint32_t cpt_index;
	uint16_t counts_per_tick[OPTO_COUNTS_PER_TICK_BUFFER_SIZE];
};

struct OptoWheel g_wheel;

/**
 * Returns the prescaler of the counters used to measure the time between "opto ticks".
 *
 * @return
 * 		the prescaler
 */
uint32_t opto_get_counter_prescaler()
{
	return COUNTER_PRESCALER;
}

/**
 * opto_get_tick_count
 * Returns the total sum of all ticks from the selected wheel, after Start.
 * @param enum wheel_selector
 * @return count of ticks
 */
uint32_t opto_get_tick_count()
{
	return g_wheel.tick_count;
}

uint32_t opto_get_counts_per_tick(uint32_t values_included_count)
{
	struct OptoWheel* wheel = &g_wheel;

	if (values_included_count > OPTO_COUNTS_PER_TICK_BUFFER_SIZE)
		values_included_count = OPTO_COUNTS_PER_TICK_BUFFER_SIZE;

	uint32_t counts_per_tick = 0;

	for (int i = 0; i < values_included_count; i++)
	{
		int index = wheel->cpt_index - i;

		if (index < 0)
			index = ((int)OPTO_COUNTS_PER_TICK_BUFFER_SIZE) + index;

		counts_per_tick += wheel->counts_per_tick[index];
	}

	return counts_per_tick / values_included_count;
}

/**
 * Initializes the Opto Module
 *
 * configures MTU3/4/6/7 with prescaler 1024 to measure the speed and the count of ticks (interrupt)
 *
 * @note
 * 		Must be called first, otherwise the module won't work properly.
 */
void opto_initialize(void) {
	//Interrupt off
	//MTU6
	ICU.IER[0x11].BIT.IEN7 = 0;	//input capture
	ICU.IER[0x12].BIT.IEN2 = 0;	//overvlow

	//PORT config
	//MTU6
	PORT9.DDR.BIT.B5 = 0;
	PORT9.DR.BIT.B5 = 0;
	PORT9.ICR.BIT.B5 = 1;

	//MTU ON
	SYSTEM.MSTPCRA.BIT.MSTPA9 = 0;
	//IR Flags löschen
	//MTU6
	ICU.IR[143].BYTE = 0;	//Ip-c
	ICU.IR[146].BYTE = 0;	//Ovf

	//Priorität
	//MTU6
	ICU.IPR[0x5c].BIT.IPR = 9;	//Priorität IC
	ICU.IPR[0x5d].BIT.IPR = 8;	//Priorität ovf

	//Stop Counter
	MTU.TSTRA.BIT.CST3 = 0;
	//MTU.TSTRA.BIT.CST4=0;
	MTU.TSTRB.BIT.CST6 = 0;
	//MTU.TSTRB.BIT.CST7 = 0;

	//MTU prescaler
	MTU6.TCR.BIT.TPSC = 5;	//ICLK/1024

	//Clock Edge Select rising edge
	MTU6.TCR.BIT.CKEG = 0;

	//Counter clear   ->cleared by TGRA compare match/input capture
	MTU6.TCR.BIT.CCLR = 2;

	//Timer Mode by Default Normal/ without buffer
	MTU6.TMDR1.BYTE = 0;

	//Timer I/O Register -> Input capture on rising edges
	MTU6.TIORH.BIT.IOB = 8;	//MTIOC6B P95 CN7

	//clear interrupt flags
	//ic
	MTU6.TSR.BIT.TGFC = 0;
	//ovf
	MTU6.TSR.BIT.TCFV = 0;

	//Interrupt enable
	MTU6.TIER.BIT.TGIEB = 1; //input capture
	MTU6.TIER.BIT.TCIEV = 1; //overflow

	//MTU6
	ICU.IER[0x11].BIT.IEN7 = 1;
	ICU.IER[0x12].BIT.IEN2 = 1;

	//Start Counter
	//MTU.TSTRA.BIT.CST3 = 1;
	//MTU.TSTRA.BIT.CST4=1;
	MTU.TSTRB.BIT.CST6 = 1;
	//MTU.TSTRB.BIT.CST7 = 1;


	g_wheel.tick_count = 0;
	g_wheel.cpt_index = 0;
	g_wheel.ovf_count = 0xFFFF;

	for (int j = 0; j < OPTO_COUNTS_PER_TICK_BUFFER_SIZE; j++)
		g_wheel.counts_per_tick[j] = 0xFFFF;

}


/**
 * Interrupt input capture -> MTU6
 * MTIOC6B P95 CN7
 */
void Excep_MTU6_TGIB6(void)
{
	struct OptoWheel* wheel = &g_wheel;

	wheel->counts_per_tick[wheel->cpt_index] = (wheel->ovf_count << 16) + MTU6.TGRB;
	wheel->tick_count++;
	wheel->ovf_count = 0;

	wheel->cpt_index = RINGBUFFER_INDEX(OPTO_COUNTS_PER_TICK_BUFFER_SIZE, wheel->cpt_index + 1);

	//Debug Opto tickcount
	//debug_ticks();
	//--------------------

	// Handle Status Registers
	MTU6.TSR.BIT.TGFB = 0;

	//if (MTU6.TSR.BIT.TGFB)
		//ICU.IR[143].BIT.IR = 1;
}

/**
 * Interrupt MTU6 Overflow
 */
void Excep_MTU6_TCIV6(void)
{
	g_wheel.ovf_count++;

	// Handle Status Registers
	MTU6.TSR.BIT.TCFV = 0;

	//if (MTU6.TSR.BIT.TCFV)
		//ICU.IR[146].BIT.IR = 1;
}


