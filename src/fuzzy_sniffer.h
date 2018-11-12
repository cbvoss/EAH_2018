/*
 * fuzzy_sniffer.h
 *
 *  Created on: Jan 6, 2017
 *      Author: Tobias Hupel
 *
 *  Tested as console application (Windows) on Jan 6, 2017 by:
 *  			Tobias Hupel
 */

#ifndef FUZZY_SNIFFER_H_
#define FUZZY_SNIFFER_H_

/**
 * Struct representing a fuzzy regulator which could be used for simple line holding.
 *
 * See function descriptions for more information.
 */
struct FuzzySniffer
{
		unsigned char ignore_pattern, left_remainder, right_remainder, max_actives;
		float servo_delta, servo_delta_sum;
};

void fuzzy_sniffer_new(struct FuzzySniffer *fuzzy_sniffer, unsigned char ignore_pattern, float servo_delta, unsigned char max_actives);

int fuzzy_sniffer_calculate_servo_delta(struct FuzzySniffer *fuzzy_sniffer, unsigned char current_ir_board_pattern);

#endif /* FUZZY_SNIFFER_H_ */
