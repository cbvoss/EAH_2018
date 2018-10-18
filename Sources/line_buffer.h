/*
 * line_buffer.h
 *
 *  Created on: Mar 11, 2017
 *      Author: Tobias Hupel
 */

#ifndef LINE_BUFFER_H_
#define LINE_BUFFER_H_

#define LINE_BUFFER_SIZE 128

struct LineBuffer
{
    float pattern_mean_values[LINE_BUFFER_SIZE];
    int index;
};

void line_buffer_new(struct LineBuffer * lineBuffer);

void line_buffer_add_pattern(struct LineBuffer * lineBuffer, unsigned char pattern);

float line_buffer_calculate_gradient(struct LineBuffer * lineBuffer, unsigned int length);

float line_buffer_calculate_average(struct LineBuffer * lineBuffer, unsigned int length);

#endif /* LINE_BUFFER_H_ */
