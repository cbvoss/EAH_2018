/*
 * adc.h
 *
 *  Created on: 19.12.2016
 *      Author: franz_2
 */

#ifndef ADC_H_
#define ADC_H_

#include "typedefine.h"

enum adc_channel_select{SUB_RESULT,
						LEFT_IR,
						RIGHT_IR
};

void adc_initialize(void);
_UBYTE adc_get_value(enum adc_channel_select selected);



#endif /* ADC_H_ */
