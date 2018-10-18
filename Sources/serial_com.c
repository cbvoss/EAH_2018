/*
 * serial_com.c
 *
 *	Module for communication through USB.
 *
 *	Features:
 *		- send strings through USB to a capable receiver
 *
 *  Created on: 16.11.2016
 *      Author: Franz Luebke
 *      		Adrian Zentgraf
 */

/*PORTS:
 * PD5 PD3
 *
 */
#include "iodefine.h"
#include "vect.h"
#include "serial_com.h"

#define SERIAL1_BUFFER_MASK 0xff
#define SERIAL1_BUFFER_SIZE 256
#define SERIAL1_BAUD 159		//9600 BAUD


volatile static char g_buffer[SERIAL1_BUFFER_SIZE];
volatile static unsigned int g_w_pos_buffer = 0;
volatile static unsigned int g_r_pos_buffer = 0;
volatile static unsigned int g_r_pos_buffer_prev2 = SERIAL1_BUFFER_MASK-1;
volatile static unsigned int g_r_pos_buffer_prev1 = SERIAL1_BUFFER_MASK;

volatile static char g_Buffer_Overflow_occured = 0;

/**
 * Initializes the module.
 *
 * @note
 * 		Must be called first, otherwise the module won't work properly.
 *
 * Asynchronous, 9600 Baud, 1 Stop Bit, no parity
 */
void serial_com_initialize(void) {
	SYSTEM.MSTPCRB.BIT.MSTPB30 = 0;
	//ICU.IR[220].BIT.IR=1;

	SCI1.SCR.BYTE = 0;
	SCI1.SMR.BYTE = 0;
	SCI1.BRR = SERIAL1_BAUD;
	volatile unsigned int i = 0;
	for (i = 0; i < 4000; i++)
		;

	//SCI1.SCR.BIT.TEIE=1;

	ICU.IER[0x1b].BIT.IEN4 = 1;
	//ICU.IER[0x1b].BIT.IEN5=1;
	ICU.IPR[0x81].BIT.IPR = 7;
}

/**
 * Reads value from buffer and increments the "read pointer".
 *
 * FIFO
 */
char pull_sci1_buffer() {
	char ret = g_buffer[g_r_pos_buffer];
	g_r_pos_buffer_prev2 = g_r_pos_buffer_prev1;
	g_r_pos_buffer_prev1 = g_r_pos_buffer;
	g_r_pos_buffer++;
	g_r_pos_buffer &= SERIAL1_BUFFER_MASK;
	return ret;
}
/**
 * Writes the value into the buffer and increments the "read pointer".
 *
 * @param c
 * 		the value to write
 */
void put_sci1_buffer(char c) {
	if (g_Buffer_Overflow_occured){
		return;
	}

	if (g_w_pos_buffer == g_r_pos_buffer_prev2){
		c = '<';
		g_Buffer_Overflow_occured = 1;
	}
	else if (g_w_pos_buffer == g_r_pos_buffer_prev1){
		return;
	}

	g_buffer[g_w_pos_buffer] = c;
	g_w_pos_buffer++;
	g_w_pos_buffer &= SERIAL1_BUFFER_MASK;
}
/**
 * Starts the transmission of the buffer.
 */
void sci1_transmit_buffer() {
	if (SCI1.SSR.BIT.TDRE) {
		SCI1.SCR.BIT.TIE = 1;
		SCI1.SCR.BIT.TE = 1;
		SCI1.TDR = pull_sci1_buffer();
	}
}
/**
 * Transmits the given string through USB.
 *
 * @param c_str
 * 		the string to transmit
 */
void serial_com_write_string(char *c_str) {
	while (*c_str) {
		put_sci1_buffer(*c_str);
		c_str++;
	}
	sci1_transmit_buffer();
}
/**
 * Interrupt
 *
 * Writes the next value in the TDR for a gapless transmission.
 */
void Excep_SCI1_TXI1(void) {
	if (g_w_pos_buffer != g_r_pos_buffer) {
		SCI1.TDR = pull_sci1_buffer();
	}
	else {
		// buffer is empty again
		g_Buffer_Overflow_occured = 0;
	}
}
