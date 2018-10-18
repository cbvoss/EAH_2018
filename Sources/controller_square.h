/*
 * controller_square.h
 *
 *  Created on: 05.01.2017
 *      Author: Tobias
 */

#ifndef CONTROLLER_SQUARE_H_
#define CONTROLLER_SQUARE_H_

/*
 * controller_square.h
 *
 *  Created on: 05.01.2017
 *      Author: Tobias Hupel
 *      		Eric Elsner
 */

void controller_square_initialize();

void controller_square_reset();

void controller_square_set_active(char active);

char controller_square_is_active();

char controller_square_has_finished();

void controller_square_update();



#endif /* CONTROLLER_SQUARE_H_ */
