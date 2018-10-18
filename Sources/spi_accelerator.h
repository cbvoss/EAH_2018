/*
 * spi_accelerator.h
 *
 *		Peripherie Module: Acceleration Sensor
 *
 *      Author: 	Andreas Chwojka
 *      Date:		2018-01-29
 *      Version:	1.0
 */

#ifndef SPI_ACCELERATOR_H_
#define SPI_ACCELERATOR_H_

/* -----------------------------------------------------------------------------
 * 						Includes
 * ---------------------------------------------------------------------------*/
#include <stdint.h>
#include "spi.h"
//#include "global_clock.h"


/* -----------------------------------------------------------------------------
 * 						Acceleration Data
 * ---------------------------------------------------------------------------*/
struct spi_acc_data {
	float x;
	float y;
	float z;
};


/* -----------------------------------------------------------------------------
 * 						Module Functions
 * ---------------------------------------------------------------------------*/

// Configuration Functions
void spi_acc_initialize ( void );
void spi_acc_reset_offset( void );
void spi_acc_set_active ( uint8_t active_var );

// Update Function (Module Update)
void spi_acc_update ( void );

// Interface to other Modules
struct spi_acc_data spi_acc_get_acceleration( void );	// read sensor data (- offset)
uint8_t spi_acc_force_read ( uint8_t sensor_id );		// read single sensor, NOW

// Internal Functions
static float spi_convert_g_to_m( uint8_t g_value );


/* -----------------------------------------------------------------------------
 * 						ADXL345 based configuration
 * ---------------------------------------------------------------------------*/

// Device I2C Address
#define ACC_ADDRESS			0x1D 	//PIN3 = HIGH, alt: 0x53 @ PIN3 = LOW

// Device Registers
#define ACC_THRESH_TAP		0x1D	// R/W	(u8)	Tap threshold, LSB=62.5mg
#define ACC_THRESH_DUR		0x21	// R/W	(s8)	Duration of value >= threshold,
									// 				LSB=625us, 0 => single/double Tap disabled
#define ACC_THRESH_LATENT	0x22	// R/W	(u8)	Latency for second Tap, LSB=1.25ms
#define ACC_THRESH_WINDOW	0x23	// R/W	(u8)	time window after first tap, LSB=1.25ms
#define ACC_THRESH_ACT		0x24	// R/W	(u8)	Threshold value for detecting activity, LSB=62.5mg
									//				0 => undesirable behavior if activity interrupt enabled
#define ACC_THRESG_INACT	0x25	// R/W	(u8)	Threshold value for detecting inactivity, LSB=62.5mg
									//				0 => undesirable behavior if inactivity interrupt enabled

#define ACC_ACT_INACT_CTL 	0x27	/* R/W	(u8)	b7		= ACT ac/dc, 0= dc -> compare act val with act threshold val
									 *									 1= ac -> compare act val with ref val on start of activity detection
									 *				b6		= ACT_x_enable
									 *				b5		= ACT_y_enable
									 *				b4		= ACT_z_enable
									 *				b3		= ACT ac/dc, 0= dc -> compare act val with inact threshold val
									 *									 1= ac -> compare act val with ref val on start of activity detection
									 *				b2		= INACT_x_enable
									 *				b1		= INACT_y_enable
									 *				b0		= INACT_z_enable
									 */
#define ACC_BW_RATE			0x2C	/* R/W	(u8)	b7..b5	= 0
									 *				b4		= Low Power Bit (1=ON, more noise, Tab. 8)
									 *				b3..b0	= Rate: Table 7 and 8 (bottom)
									 */
#define ACC_POWER_CTL		0x2D	/* R/W	(u8)	b7..b6 	= 0
									 *				b5		= Link activity detection -> inactivity det.
									 *				b4		= Auto Sleep if in inactivity state
									 *				b3		= Measure, 0 => device in standby mode (default)
									 *				b2		= Sleep, 0 => normal mode, 1 => no data transmission
									 *				b1..b0	= Wakeup: Reading frequency in standby mode
									 *						=> 00 = 8 Hz
									 *						=> 01 = 4 Hz
									 *						=> 10 = 2 Hz
									 *						=> 11 = 1 Hz
									 */
#define ACC_INT_ENABLE		0x2E	/* R/W	(u8)	Enable Interrupts
									 *				b7		= DATA_READY
									 *				b6		= SINGLE_TAP
									 *				b5		= DOUBLE_TAP
									 *				b4		= Activity
									 *				b3		= Inactivity
									 *				b2		= FREE_FALL
									 *				b1		= Watermark
									 *				b0		= Overrun
									 */
#define ACC_INT_MAP			0x2F	/* R/W	(u8)	Map Interrupts to PIN INT0 if 0 was set and
									 * 				to INT1 if 1 was set
									 *				b7		= DATA_READY
									 *				b6		= SINGLE_TAP
									 *				b5		= DOUBLE_TAP
									 *				b4		= Activity
									 *				b3		= Inactivity
									 *				b2		= FREE_FALL
									 *				b1		= Watermark
									 *				b0		= Overrun
									 */
#define ACC_INT_SOURCE		0x30	/* R	(u8)	Bits set to 1 if function triggered an event
									 *				b7		= DATA_READY
									 *				b6		= SINGLE_TAP
									 *				b5		= DOUBLE_TAP
									 *				b4		= Activity
									 *				b3		= Inactivity
									 *				b2		= FREE_FALL
									 *				b1		= Watermark
									 *				b0		= Overrun
									 */
#define ACC_DATA_FORMAT		0x31	/* R/W	(u8)	Controls presentation of data in REG 0x32 .. 0x37
									 *				b7		= SELF_TEST, 1 => shift in output data
									 *				b6		= SPI, 0 => 4-wire SPI, 1 => 3-wire SPI
									 *				b5		= INT_INVERT, 0 => Interrupts active HIGH, 1 => LOW
									 *				b4		= 0
									 *				b3		= FULL_RES, 0 => 10bit Mode, 1 => 4 mg/LSB * g Range
									 *				b2		= Justify, 1 => left MSB, 0 =>
									 *				b1..b0	= Range
									 *						=>	00 = +/-  2g
									 *						=>	01 = +/-  4g
									 *						=>	10 = +/-  8g
									 *						=>	11 = +/- 16g
									 */

#define ACC_FIFO_CTL		0x38	/* R/W	(u8)	b7..b6 	= FIFO Mode
								*							=> 	00	Bypass
								*							=>	01	FIFO (up to 32 val), stop if 32 val collectet
								*							=>	10	Stream, like FIFO, but oldest val is overwritten
								*							=>	11	Trigger, collect until triger value, then triger bit is set and stops
								*					b5		= Trigger Bit, 0 trigger => INT1, 1 => INT2
								*					b4..b0	= Sample Bits, depends on b7..b6
								*							=> Bypass: NoNe
								*							=> FIFO: entries until trigger watermark INT
								*							=> Stream: entries until trigger watermark INT
								*							=> Trigger: entries until trigger event
								*/

#define ACC_FIFO_STATUS		0x39	/* R	(u8)	b7		= FIFO Trig Bit, 1 on trigger event
													b6		= 0
													b5..b0	= Entries in FIFO memory
									*/


// Offsets
#define ACC_OFSX			0x1E	// R/W	(s8)	X-axis Offset
#define ACC_OFSY			0x1F	// R/W	(s8)	Y-axis Offset
#define ACC_OFSZ			0x20	// R/W	(s8)	Z-axis Offset

// Data, x0= LSB, x1= MSB
#define ACC_DATAX0			0x32	// R	(s8)	X-Axis Data 0
#define ACC_DATAX1			0x33	// R	(s8)	X-Axis Data 1
#define ACC_DATAY0			0x34	// R	(s8)	Y-Axis Data 0
#define ACC_DATAY1			0x35	// R	(s8)	Y-Axis Data 1
#define ACC_DATAZ0			0x36	// R	(s8)	Z-Axis Data 0
#define ACC_DATAZ1			0x37	// R	(s8)	Z-Axis Data 1


/*
 * Configuration Tables
 * --------------------
 *
 * Table 7
 * -------
 * (TA = 25°C, VS = 2.5 V, VDD I/O = 1.8 V)
 * Output Data
 * Rate (Hz) 	Bandwidth (Hz) 	Rate Code 	IDD (µA)
 *
 * 3200 		1600 			1111 		140
 * 1600 		 800 			1110 		 90
 *  800 		 400 			1101 		140
 *  400 		 200 			1100 		140
 *  200 		 100 			1011 		140		* selected
 *  100			  50 			1010 		140
 *   50 		  25 			1001 		 90
 *   25 		  12.5 			1000 		 60
 * 	 12.5 		  6.25 			0111 		 50
 * 	  6.25 		  3.13 			0110 		 45
 * 	  3.13 		  1.56 			0101 		 40
 *    1.56 		  0.78 			0100 		 34
 *    0.78 		  0.39 			0011 		 23
 *    0.39 		  0.20 			0010 		 23
 *    0.20 		  0.10 			0001 		 23
 *    0.10 		  0.05 			0000 		 23
 *
 *
 *
 * Table 8, Low Power Mode
 * -------
 * (TA = 25°C, VS = 2.5 V, VDD I/O = 1.8 V)
 * Output Data
 * Rate (Hz) 	Bandwidth (Hz) 	Rate Code 	IDD (µA)
 *
 *  400 		200 			1100 		 90
 *  200 		100 			1011 		 60
 *  100 		 50 			1010 		 50
 *   50 		 25 			1001 		 45
 *   25 		 12.5 			1000 		 40
 *   12.5 		  6.25 			0111 		 34
 *
 *
*/

#endif /* spi_accELERATOR_H_ */
