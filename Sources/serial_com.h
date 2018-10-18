/*
 * serial_com.h
 *
 *  Module for communication through USB.
 *
 *	Features:
 *		- send strings through USB to a capable receiver
 *
 *  Created on: 16.11.2016
 *      Author: Franz Luebke
 *      		Adrian Zentgraf
 */

#ifndef SERIAL_COM_H_
#define SERIAL_COM_H_

void serial_com_initialize(void);
void serial_com_write_string(char *c_str);

#endif /* SERIAL_COM_H_ */
