/*
 * engine.h
 *
 *	Hardware Module for engine controlling.
 *
 *  Created on: 6 Nov 2016
 *      Author: Tobias Hupel
 *
 *  Reviewed by:
 *  			Florian Schaedlich
 *  			Tobias Hupel
 */

#ifndef HARDWARE_ENGINE_H_
#define HARDWARE_ENGINE_H_

/**
 * Enum, representing the operation mode of the engine.
 */
enum OperationMode {
	FORWARD_FREERUN, BACKWARD_FREERUN, FORWARD_BREAK, BACKWARD_BREAK, FREERUN, BREAK
};

void engine_initialize();

void engine_set_pulse_width_pm(enum Side side, unsigned int pulse_width);

unsigned int engine_get_pulse_width_pm(enum Side side);

void engine_set_mode(enum Side side, enum OperationMode mode);

enum OperationMode engine_get_mode(enum Side side);

#endif /* HARDWARE_ENGINE_H_ */
