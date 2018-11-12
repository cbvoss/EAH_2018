/*
 * ir_pattern_tools.h
 *
 *  Created on: Dec 7, 2017
 *      Author: Tobias Hupel
 */

#ifndef TOOLS_IR_PATTERN_TOOLS_H_
#define TOOLS_IR_PATTERN_TOOLS_H_

typedef unsigned char IRPattern;

float calculate_ones_mean_value(IRPattern pattern);

char contains_single_continuous_ones_row(IRPattern pattern);

unsigned int get_ones_count(IRPattern pattern);

#endif /* TOOLS_IR_PATTERN_TOOLS_H_ */
