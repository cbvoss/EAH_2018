/*
 * Ir_Board.h
 *
 *  Created on: 05.11.2016
 *      Author: Eric Elsner
 */

#ifndef IR_BOARD_H_
#define IR_BOARD_H_

char ir_get_value(char elementIndex);
unsigned char ir_get_pattern();
char ir_get_front_sensor();
void ir_initialize();

#endif /* IR_BOARD_H_ */
