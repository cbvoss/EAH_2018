/*
 * adc.c
 *
 *  Created on: 15.12.2016
 *      Author: franz lubke
 *      		adrian zentgraf
 */


#include "iodefine.h"
#include "vect.h"
#include "adc.h"
#include "config_car.h"
void adc_start_convertion(void);
/**
 * Initialize the 10bit ADC
 * on port P50 / AD6
 */
void adc_initialize(void) {

	// Power up 10bit ADC
	SYSTEM.MSTPCRA.BIT.MSTPA23 = 0;

	// PCLK/8
	AD0.ADCR.BIT.CKS = 0;

	// Set Continuous Scan Mode
	AD0.ADCR.BIT.MODE = 2;

	// Set Sampling Time to 4us
	AD0.ADSSTR = 0x19;

	// Channel AN6,7,8,9 used (Continuous scan from AN0 to AN9)
	AD0.ADCSR.BIT.CH = 9;

	// set 8bit accuracy
	AD0.ADDPR.BIT.DPPRC = 1;
	AD0.ADCSR.BIT.ADST = 0;
	adc_start_convertion();
}
/**
 * starts the ADC
 */
void adc_start_convertion(void){
	//starts conversion if no current conversion
	//if(!AD0.ADCSR.BIT.ADST) AD0.ADCSR.BIT.ADST=1;
	AD0.ADCSR.BIT.ADST=1;
}
/**
 * adc at 0V 0x00
 * adc at 5V 0xff
 *
 * @return adc value
 */

_UBYTE adc_get_value(enum adc_channel_select selected){
	_UBYTE adc_value=0;
	switch(selected) {
		case SUB_RESULT: adc_value = SUB_REG; break;
		case LEFT_IR: adc_value = LEFT_IR_REG; break;
		case RIGHT_IR: adc_value = RIGHT_IR_REG; break;
		default: break;
	}
	return (_UBYTE)adc_value;
}
