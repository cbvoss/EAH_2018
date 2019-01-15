/*
 * opto_interruptor.h
 *
 *	Driver to use opto sensors
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
 *     	Modified on 15.01.2019 by:
 *     			Heiko Gericke
 */

#ifndef OPTO_INTERRUPTOR_H_
#define OPTO_INTERRUPTOR_H_

#include <stdint.h>
#include "typedefine.h"
#include "enums.h"

#define OPTO_COUNTS_PER_TICK_BUFFER_SIZE 8

uint32_t opto_get_tick_count(enum wheel_selector wheel);

uint32_t opto_get_counts_per_tick(enum wheel_selector selector, uint32_t values_included_count);

uint32_t opto_get_counter_prescaler();

void opto_initialize(void);

#endif /* OPTO_INTERRUPTOR_H_ */
