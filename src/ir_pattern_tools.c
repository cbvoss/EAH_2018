/*
 * ir_pattern_tools.c
 *
 *  Created on: Dec 7, 2017
 *      Author: Tobias Hupel
 */

#include "ir_pattern_tools.h"

float calculate_ones_mean_value(IRPattern pattern)
{
	float meanValue = 0;
	unsigned int onesCount = get_ones_count(pattern);

	for (int i = 0; i <= 7; i++)
	{
		if (pattern & (1 << i))
		{
			meanValue += i;
		}
	}

	return meanValue / onesCount;
}

char contains_single_continuous_ones_row(IRPattern pattern)
{
	unsigned int toggles = 0;
	char lastValue = 0;

	for (int i = 0; i <= 7 && toggles <= 2; i++)
	{
		char currentValue = (pattern & (1 << i)) != 0;

		if (currentValue != lastValue)
		{
			lastValue = currentValue;
			toggles++;
		}
	}

	return toggles <= 2;
}

unsigned int get_ones_count(IRPattern pattern)
{
	unsigned int onesCount = 0;

	for (int i = 0; i <= 7; i++)
	{
		if (pattern & (1 << i))
		{
			onesCount++;
		}
	}

	return onesCount;
}
