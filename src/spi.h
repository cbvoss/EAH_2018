/*
 * spi.h
 *
 *		Module: SPI Bus Controller
 *
 *      Author: 	Andreas Chwojka
 *      Date:		2018-01-29
 *      Version:	1.0
 *
 *      Changes:	* first release
 *
 */

#ifndef SPI_H_
#define SPI_H_

/* -----------------------------------------------------------------------------
 * 						Includes
 * ---------------------------------------------------------------------------*/
#include <stdint.h>
#include "typedefine.h"
#include "global_clock.h"

/* -----------------------------------------------------------------------------
 * 						Public Functions
 * ---------------------------------------------------------------------------*/
void spi_initialize ( void );
uint8_t spi_data ( void );
uint8_t spi_finished ( void );
void spi_read ( uint8_t address );
void spi_send ( uint16_t data );

#endif
