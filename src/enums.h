/*
 * enums.h
 *
 *	This file should contain only enumerations for general purposes.
 *
 *  Created on: 6 Nov 2016
 *      Author: Tobias Hupel
 */

#ifndef ENUMS_H_
#define ENUMS_H_



/**
 * Enum, representing a side.
 */
enum Side {
	RIGHT, LEFT
};

/**wheel_selector
 * enum type to specify the interested Wheel
 *      __
 *     /  \
 * FL O|  |O FR
 *     |  |
 * BL O----O BR
 */
enum wheel_selector{FRONT_LEFT,
					FRONT_RIGHT,
					BACK_LEFT,
					BACK_RIGHT
};


/**
 * Track_Events
 * NONE --> Line tracking
 * JUMP --> Change of Lane
 * SQUARE --> 90 degree curve
 */
enum track_event{NONE,
				 JUMP_LEFT,
				 JUMP_RIGHT,
				 SQUARE,
				 SLOW
				 };

#endif /* ENUMS_H_ */
