/*
 * fuzzy_sniffer.c
 *
 *  Created on: Jan 6, 2017
 *      Author: Tobias Hupel
 *
 *  Tested as console application (Windows) on Jan 6, 2017 by:
 *  			Tobias Hupel
 */

#include "fuzzy_sniffer.h"
#include <math.h>

/**
 * Initializes a new fuzzy sniffer instance with the given values.
 *
 * @param fuzzy_sniffer
 * 		the fuzzy sniffer to be initialized
 * @param ignore_pattern
 * 		specifies the ir board pattern in which the fuzzy sniffer does nothing (returns zero servo delta on calculation)
 * 		the pattern may only contain one line of one's and at least one zero, e.g:
 * 		valid patterns: 00011000, 1110000, 01111111 invalid patterns: 01101000, 00101110, 11111111
 * @param servo_delta
 * 		the servo delta (negative or positive depends on side) returned if the ignore pattern does not include the current ir board pattern
 * @param max_actives
 * 		the maximum count of active LEDs (ones), returned delta will be '0' if the current ir board pattern has more active LEDs (ones)
 */
void fuzzy_sniffer_new(struct FuzzySniffer *fuzzy_sniffer, unsigned char ignore_pattern, float servo_delta, unsigned char max_actives)
{
	unsigned char *remainder, index, mask;

	fuzzy_sniffer->ignore_pattern = ignore_pattern;
	fuzzy_sniffer->servo_delta = fabsf(servo_delta);
	fuzzy_sniffer->servo_delta_sum = 0;
	fuzzy_sniffer->right_remainder = 0;
	fuzzy_sniffer->left_remainder = 0;
	fuzzy_sniffer->max_actives = max_actives;

	remainder = &fuzzy_sniffer->right_remainder;

	for (index = 0, mask = 1; index < 8; index++, mask = mask << 1)
	{
		if (!(mask & ignore_pattern))
		{
			*remainder += 1 << index;
		}
		else
		{
			remainder = &fuzzy_sniffer->left_remainder;
		}
	}
}

/**
 * Calculates the servo delta depending on the current ir board pattern.
 *
 * @param fuzzy_sniffer
 * 		the fuzzy sniffer providing the servo delta and the ir board pattern
 * @param current_ir_board_pattern
 * 		the current ir board pattern
 * @return
 * 		the servo delta calculated with the fuzzy sniffer's ignore pattern
 */
int fuzzy_sniffer_calculate_servo_delta(struct FuzzySniffer *fuzzy_sniffer, unsigned char current_ir_board_pattern)
{
	int servo_delta = 0, actives = 0, i;
	unsigned char mask;

	for (i = 0; i < 8; i++)
	{
		mask = 1 << i;

		if (mask & current_ir_board_pattern)
			actives++;
	}

	if (actives <= fuzzy_sniffer->max_actives)
	{
		if (current_ir_board_pattern & ~(fuzzy_sniffer->ignore_pattern + fuzzy_sniffer->right_remainder))
		{
			fuzzy_sniffer->servo_delta_sum += fuzzy_sniffer->servo_delta;
		}
		if (current_ir_board_pattern & ~(fuzzy_sniffer->ignore_pattern + fuzzy_sniffer->left_remainder))
		{
			fuzzy_sniffer->servo_delta_sum -= fuzzy_sniffer->servo_delta;
		}

		if (fabsf(fuzzy_sniffer->servo_delta_sum) >= 1.0f)
		{
			servo_delta = (int) fuzzy_sniffer->servo_delta_sum;
			fuzzy_sniffer->servo_delta_sum = 0;
		}
	}

	return servo_delta;
}

