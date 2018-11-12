/*
 * hall.c
 *
 *	Implementation of Driver to use Hall sensors
 *
 *  Created on: 12.11.2016
 *      Author: Franz Lübke
 *      		Adrian Zentgraf
 *
 *      Reviewed on 12.02.2016 by:
 *      		Franz Lübke
 *      		Eric Elsner
 *      		Tobias Hupel
 */
#include "hall.h"
#include "iodefine.h"
#include "vect.h"

#define COUNTER_PRESCALER 1024			// Prescaler of counters
#define WHEEL_COUNT 4

#define RINGBUFFER_INDEX(LENGTH, OFFSET) ((OFFSET) % (LENGTH))

/**
   Global variables Hall
 * sum Counts of wheel ticks
 */
struct HallWheel
{
	uint32_t tick_count;
	uint32_t ovf_count;
	uint32_t cpt_index;
	uint16_t counts_per_tick[HALL_COUNTS_PER_TICK_BUFFER_SIZE];
};

struct HallWheel g_wheels[WHEEL_COUNT];

/**
 * Returns the prescaler of the counters used to measure the time between "hall ticks".
 *
 * @return
 * 		the prescaler
 */
uint32_t hall_get_counter_prescaler()
{
	return COUNTER_PRESCALER;
}

/**
 * hall_get_tick_count
 * Returns the total sum of all ticks from the selected wheel, after Start.
 * @param enum wheel_selector
 * @return count of ticks
 */
uint32_t hall_get_tick_count(enum wheel_selector wheel)
{
	return g_wheels[wheel].tick_count;
}

uint32_t hall_get_counts_per_tick(enum wheel_selector selector, uint32_t values_included_count)
{
	struct HallWheel* wheel = &g_wheels[selector];

	if (values_included_count > HALL_COUNTS_PER_TICK_BUFFER_SIZE)
		values_included_count = HALL_COUNTS_PER_TICK_BUFFER_SIZE;

	uint32_t counts_per_tick = 0;

	for (int i = 0; i < values_included_count; i++)
	{
		int index = wheel->cpt_index - i;

		if (index < 0)
			index = ((int)HALL_COUNTS_PER_TICK_BUFFER_SIZE) + index;

		counts_per_tick += wheel->counts_per_tick[index];
	}

	return counts_per_tick / values_included_count;
}

/**
 * Initializes the Hall Module
 *
 * configures MTU3/4/6/7 with prescaler 1024 to measure the speed and the count of ticks (interrupt)
 *
 * @note
 * 		Must be called first, otherwise the module won't work properly.
 */
void hall_initialize(void) {
	//Interrupt off
	//MTU3
	ICU.IER[0x10].BIT.IEN3 = 0;	//input capture
	ICU.IER[0x10].BIT.IEN5 = 0;	//overvlow
	//MTU6
	ICU.IER[0x11].BIT.IEN7 = 0;	//input capture
	ICU.IER[0x12].BIT.IEN2 = 0;	//overvlow
	//MTU7
	//ICU.IER[0x12].BIT.IEN6 = 0;	//input capture
	//ICU.IER[0x13].BIT.IEN1 = 0;	//overvlow

	//PORT config
	//MTU3
	PORT3.DDR.BIT.B2 = 0;
	PORT3.DR.BIT.B2 = 0;
	PORT3.ICR.BIT.B2 = 1;
	//MTU6
	PORT9.DDR.BIT.B5 = 0;
	PORT9.DR.BIT.B5 = 0;
	PORT9.ICR.BIT.B5 = 1;
	//MTU7
	//PORT9.DDR.BIT.B3 = 0;
	//PORT9.DR.BIT.B3 = 0;
	//PORT9.ICR.BIT.B3 = 1;

	//MTU ON
	SYSTEM.MSTPCRA.BIT.MSTPA9 = 0;
	//IR Flags löschen
	//MTU3
	ICU.IR[133].BYTE = 0;	//MTU3 Ovf
	ICU.IR[131].BYTE = 0;	//MTU3 Ip-c
	//MTU6
	ICU.IR[143].BYTE = 0;	//Ip-c
	ICU.IR[146].BYTE = 0;	//Ovf
	//MTU7
	//ICU.IR[150].BYTE = 0;	//Ip-c
	//ICU.IR[153].BYTE = 0;	//Ovf

	//Priorität
	//MTU3
	ICU.IPR[0x57].BIT.IPR = 9;	//Priorität IC
	ICU.IPR[0x58].BIT.IPR = 8;	//Priorität ovf
	//MTU6
	ICU.IPR[0x5c].BIT.IPR = 9;	//Priorität IC
	ICU.IPR[0x5d].BIT.IPR = 8;	//Priorität ovf
	//MTU7
	//ICU.IPR[0x5e].BIT.IPR = 9;	//Priorität IC
	//ICU.IPR[0x60].BIT.IPR = 8;	//Priorität ovf
	//Stop Counter
	MTU.TSTRA.BIT.CST3 = 0;
	//MTU.TSTRA.BIT.CST4=0;
	MTU.TSTRB.BIT.CST6 = 0;
	//MTU.TSTRB.BIT.CST7 = 0;

	//MTU prescaler
	MTU3.TCR.BIT.TPSC = 5;	//ICLK/1024
	//MTU4.TCR.BIT.TPSC=5;	//ICLK/1024
	MTU6.TCR.BIT.TPSC = 5;	//ICLK/1024
	//MTU7.TCR.BIT.TPSC = 5;	//ICLK/1024
	//Clock Edge Select rising edge
	MTU3.TCR.BIT.CKEG = 0;
	//MTU4.TCR.BIT.CKEG=0;
	MTU6.TCR.BIT.CKEG = 0;
	//MTU7.TCR.BIT.CKEG = 0;
	//Counter clear   ->cleared by TGRA compare match/input capture
	MTU3.TCR.BIT.CCLR = 5;
	//MTU4.TCR.BIT.CCLR=1;
	MTU6.TCR.BIT.CCLR = 2;
	//MTU7.TCR.BIT.CCLR = 2;

	//Timer Mode by Default Normal/ without buffer
	MTU3.TMDR1.BYTE = 0;
	MTU6.TMDR1.BYTE = 0;
	//MTU7.TMDR1.BYTE = 0;

	//Timer I/O Register -> Input capture on rising edges
	MTU3.TIORL.BIT.IOC = 8;	//MTIOC3C  P32 CN1
	//MTU4.TIORH.BIT.IOB=8;	//MTIOC4B P73 CN3 //


	MTU6.TIORH.BIT.IOB = 8;	//MTIOC6B P95 CN7
	//MTU7.TIORH.BIT.IOA = 8;	//MTIOC7B P93 CN7

	//clear interrupt flags
	//ic
	MTU3.TSR.BIT.TGFC = 0;
	MTU6.TSR.BIT.TGFC = 0;
	//MTU7.TSR.BIT.TGFC = 0;
	//ovf
	MTU3.TSR.BIT.TCFV = 0;
	MTU6.TSR.BIT.TCFV = 0;
	//MTU7.TSR.BIT.TCFV = 0;

	//Interrupt enable
	MTU3.TIER.BIT.TGIEC = 1; //input capture
	MTU3.TIER.BIT.TCIEV = 1; //overflow
//	MTU4.TIER.BIT.TGIEB=1; //input capture
//	MTU4.TIER.BIT.TCIEV=1; //overflow
	MTU6.TIER.BIT.TGIEB = 1; //input capture
	MTU6.TIER.BIT.TCIEV = 1; //overflow
	//MTU7.TIER.BIT.TGIEB = 1; //input capture
	//MTU7.TIER.BIT.TCIEV = 1;  //overflow

	//MTU3
	ICU.IER[0x10].BIT.IEN3 = 1;
	ICU.IER[0x10].BIT.IEN5 = 1;
	//MTU6
	ICU.IER[0x11].BIT.IEN7 = 1;
	ICU.IER[0x12].BIT.IEN2 = 1;
	//MTU7
	//ICU.IER[0x12].BIT.IEN6 = 1;
	//ICU.IER[0x12].BIT.IEN2 = 1;

	//Start Counter
	MTU.TSTRA.BIT.CST3 = 1;
	//MTU.TSTRA.BIT.CST4=1;
	MTU.TSTRB.BIT.CST6 = 1;
	//MTU.TSTRB.BIT.CST7 = 1;

	for (int i = 0; i < WHEEL_COUNT; i++)
	{
		g_wheels[i].tick_count = 0;
		g_wheels[i].cpt_index = 0;
		g_wheels[i].ovf_count = 0xFFFF;

		for (int j = 0; j < HALL_COUNTS_PER_TICK_BUFFER_SIZE; j++)
			g_wheels[i].counts_per_tick[j] = 0xFFFF;
	}
}

/**
 * Interrupt input capture -> MTU3
 * MTIOC3B P71 CN3
 */
void Excep_MTU3_TGIC3(void)
{
	struct HallWheel* wheel = &g_wheels[BACK_LEFT];

	wheel->counts_per_tick[wheel->cpt_index] = (wheel->ovf_count << 16) + MTU3.TGRC;
	wheel->tick_count++;
	wheel->ovf_count = 0;

	wheel->cpt_index = RINGBUFFER_INDEX(HALL_COUNTS_PER_TICK_BUFFER_SIZE, wheel->cpt_index + 1);

	// Handle Status Registers
	MTU3.TSR.BIT.TGFC = 0;

	if (MTU3.TSR.BIT.TGFC)
		ICU.IR[131].BIT.IR = 1;
}

/**
 * Interrupt MTU3 Overflow
 */
void Excep_MTU3_TCIV3(void)
{
	g_wheels[BACK_LEFT].ovf_count++;

	// Handle Status Registers
	MTU3.TSR.BIT.TCFV = 0;

	if (MTU3.TSR.BIT.TCFV)
		ICU.IR[133].BIT.IR = 1;
}
/**
 * Interrupt input capture -> MTU4
 * MTIOC4B P73 CN3
 */
void Excep_MTU4_TGIB4(void) {
	/*g_hall_wheel.BACK_RIGHT.counts_per_tick=g_hall_wheel.BACK_LEFT.ovf_count*0xFFFF+MTU4.TGRB;
	 g_hall_wheel.BACK_RIGHT.tick_count++;
	 g_hall_wheel.BACK_RIGHT.ovf_count=0;
	 MTU4.TSR.BIT.TGFC=0;
	 */
}

/**
 * Interrupt MTU4 Overflow
 */
void Excep_MTU4_TCIV4(void) {
	/*
	 g_hall_wheel.BACK_RIGHT.ovf_count++;
	 MTU4.TSR.BIT.TCFV=0;
	 */
}
/**
 * Interrupt input capture -> MTU6
 * MTIOC6B P95 CN7
 */
void Excep_MTU6_TGIB6(void)
{
	struct HallWheel* wheel = &g_wheels[BACK_RIGHT];

	wheel->counts_per_tick[wheel->cpt_index] = (wheel->ovf_count << 16) + MTU6.TGRB;
	wheel->tick_count++;
	wheel->ovf_count = 0;

	wheel->cpt_index = RINGBUFFER_INDEX(HALL_COUNTS_PER_TICK_BUFFER_SIZE, wheel->cpt_index + 1);

	// Handle Status Registers
	MTU6.TSR.BIT.TGFB = 0;

	if (MTU6.TSR.BIT.TGFB)
		ICU.IR[143].BIT.IR = 1;
}

/**
 * Interrupt MTU6 Overflow
 */
void Excep_MTU6_TCIV6(void)
{
	g_wheels[BACK_RIGHT].ovf_count++;

	// Handle Status Registers
	MTU6.TSR.BIT.TCFV = 0;

	if (MTU6.TSR.BIT.TCFV)
		ICU.IR[146].BIT.IR = 1;
}
/**
 * Interrupt input capture -> MTU7
 * MTIOC7B P93 CN7
 */
void Excep_MTU7_TGIB7(void) {/*
	g_hall_wheel.FRONT_RIGTH.counts_per_tick = (g_hall_wheel.FRONT_RIGTH.ovf_count << 16) + MTU7.TGRB;
	g_hall_wheel.FRONT_RIGTH.tick_count++;
	g_hall_wheel.FRONT_RIGTH.ovf_count = 0;
	MTU7.TSR.BIT.TGFB = 0;
*/}

/**
 * Interrupt MTU7 Overflow
 */
void Excep_MTU7_TCIV7(void) {/*
	g_hall_wheel.FRONT_RIGTH.ovf_count++;
	MTU7.TSR.BIT.TCFV = 0;
*/}

