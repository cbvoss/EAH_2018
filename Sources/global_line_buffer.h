/*
 * global_line_buffer.h
 *
 *  Created on: Mar 11, 2017
 *      Author: Tobias Hupel
 */

#ifndef GLOBAL_LINE_BUFFER_H_
#define GLOBAL_LINE_BUFFER_H_

void global_line_buffer_initialize();

void global_line_buffer_update();

float global_line_buffer_calculate_gradient(unsigned int length);

float global_line_buffer_calculate_average(unsigned int length);

#endif /* GLOBAL_LINE_BUFFER_H_ */
