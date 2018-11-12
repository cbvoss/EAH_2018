/*
 * hall.h
 *
 *	Driver to use Hall sensors
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

#ifndef HALL_H_
#define HALL_H_

#include <stdint.h>
#include "typedefine.h"
#include "enums.h"

#define HALL_COUNTS_PER_TICK_BUFFER_SIZE 8

uint32_t hall_get_tick_count(enum wheel_selector wheel);

uint32_t hall_get_counts_per_tick(enum wheel_selector selector, uint32_t values_included_count);

uint32_t hall_get_counter_prescaler();

void hall_initialize(void);

#endif /* HALL_H_ */
