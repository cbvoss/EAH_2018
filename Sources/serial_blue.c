/*
 * serial_blue.c
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

#include "iodefine.h"
#include "vect.h"
#include "serial_blue.h"

#define SERIAL2_BUFFER_MASK 0xfff
#define SERIAL2_BUFFER_SIZE 4096
#define SERIAL2_BAUD 12		//9600 BAUD->159 // 115200->12


volatile  char g_blue_buffer[SERIAL2_BUFFER_SIZE];
volatile  unsigned int g_blue_w_pos_buffer = 0;
volatile  unsigned int g_blue_r_pos_buffer = 0;
volatile int g_blue_overflow_detect=0;

#define SERIAL_BLUE_BUFFER_SIZE 256
#define SERIAL_BLUE_BUFFER_MASK 0x00ff

volatile char g_blue_resive_buffer[SERIAL_BLUE_BUFFER_SIZE];
volatile unsigned int g_blue_res_w_pos_buffer = 0;
volatile unsigned int g_blue_res_r_pos_buffer = 0;
volatile int g_blue_res_overflow_detect=0;
volatile char g_wait_until_buffer_empty = 0;


/**
 * Initializes the module.
 *
 * @note
 * 		Must be called first, otherwise the module won't work properly.
 *
 * Asynchronous, 9600 Baud, 1 Stop Bit, no parity
 */
void serial_blue_initialize(void) {
	SYSTEM.MSTPCRB.BIT.MSTPB29 = 0;
	IOPORT.PFFSCI.BIT.SCI2S = 1;
	//ICU.IR[220].BIT.IR=1;

	SCI2.SCR.BYTE = 0;
	SCI2.SMR.BYTE = 0;
	PORT8.ICR.BIT.B0=1;
	SCI2.BRR = SERIAL2_BAUD;
	volatile unsigned int i = 0;
	for (i = 0; i < 4000; i++);

	//SCI1.SCR.BIT.TEIE=1;

	ICU.IER[0x1c].BIT.IEN0 = 1;
	ICU.IER[0x1b].BIT.IEN7 = 1;
	//ICU.IER[0x1b].BIT.IEN5=1;
	ICU.IPR[0x82].BIT.IPR = 8;
	SCI2.SCR.BIT.TIE = 1;
	SCI2.SCR.BIT.TE = 1;
	//empfangen
	SCI2.SCR.BIT.RIE=1;
	SCI2.SCR.BIT.RE=1;

}

/**
 * Reads value from buffer and increments the "read pointer".
 *
 * FIFO
 */
char serial_blue_pull_receive_buffer() {
	if(g_blue_res_overflow_detect>0){
		g_blue_res_overflow_detect--;
		char ret = g_blue_resive_buffer[g_blue_res_r_pos_buffer];
		g_blue_res_r_pos_buffer++;
		g_blue_res_r_pos_buffer &= SERIAL_BLUE_BUFFER_MASK;
		return ret;
	}else return 0;
}
/**
 * Writes the value into the buffer and increments the "read pointer".
 *
 * @param c
 * 		the value to write
 */
void serial_blue_put_resive_buffer(char c) {
	if(g_blue_res_overflow_detect<SERIAL_BLUE_BUFFER_SIZE){
		g_blue_res_overflow_detect++;
		g_blue_resive_buffer[g_blue_res_w_pos_buffer] = c;
		g_blue_res_w_pos_buffer++;
		g_blue_res_w_pos_buffer &= SERIAL_BLUE_BUFFER_MASK;
	}
}

/**
 * Reads value from buffer and increments the "read pointer".
 *
 * FIFO
 */
char pull_sci2_buffer() {
	if(g_blue_overflow_detect == 0)
		return 0;

	g_blue_overflow_detect--;
	char ret = g_blue_buffer[g_blue_r_pos_buffer];
	g_blue_r_pos_buffer++;
	g_blue_r_pos_buffer &= SERIAL2_BUFFER_MASK;

	if (g_blue_overflow_detect == 0)
		g_wait_until_buffer_empty = 0;

	return ret;
}
/**
 * Writes the value into the buffer and increments the "read pointer".
 *
 * @param c
 * 		the value to write
 */
char put_sci2_buffer(char c) {
	if (g_wait_until_buffer_empty != 0)
		return 0;

	if ((g_blue_overflow_detect+1)==SERIAL2_BUFFER_SIZE){
		g_wait_until_buffer_empty = 1;
		c = '<';	// mark buffer overflow
	}

	g_blue_overflow_detect++;
	g_blue_buffer[g_blue_w_pos_buffer] = c;
	g_blue_w_pos_buffer++;
	g_blue_w_pos_buffer &= SERIAL2_BUFFER_MASK;
	return 1;
}
/**
 * Starts the transmission of the buffer.
 */
void sci2_transmit_buffer() {
	if (SCI2.SSR.BIT.TDRE) {
		SCI2.SSR.BIT.FER=0;
		SCI2.SSR.BIT.ORER=0;
		SCI2.SSR.BIT.PER=0;
		SCI2.TDR = pull_sci2_buffer();
	}
}
/**
 * Transmits the given string through USB.
 *
 * @param c_str
 * 		the string to transmit
 */
void serial_blue_write_string(char *c_str) {
	while (*c_str) {
		put_sci2_buffer(*c_str);
		c_str++;
	}
	sci2_transmit_buffer();
}
/**
 * Interrupt
 *
 * Writes the next value in the TDR for a gapless transmission.
 */
void Excep_SCI2_TXI2(void) {
	char ret = pull_sci2_buffer();
	if (ret){
		SCI2.SSR.BIT.FER=0;
		SCI2.SSR.BIT.ORER=0;
		SCI2.SSR.BIT.PER=0;
		SCI2.TDR = ret;
	}
}
void Excep_SCI2_RXI2(void){
	serial_blue_put_resive_buffer(SCI2.RDR);
}
