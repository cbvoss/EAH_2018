/*
 * controller_jump.h
 *
 * is the controller who performs the line change.
 *
 *  Created on: 01.01.2017
 *      Author: franz_lübke
 */

#ifndef CONTROLLER_JUMP_H_
#define CONTROLLER_JUMP_H_
#include "enums.h"

void controller_jump_update(void);
void controller_jump_initialize(void);
void controller_jump_set_jump_side(enum Side side);
void controller_jump_set_active(char active);
char controller_jump_has_finished();
void controller_jump_reset();

#endif /* CONTROLLER_JUMP_H_ */
