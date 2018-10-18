/*
 * spi.c
 *
 * 		Module: SPI Bus Controller
 *
 *      Author: 	Andreas Chwojka
 *      Version:	1.1
 *      Date:		2018-02-19
 *
 *      Dependencies:
 *      -	Global Clock Timer
 *
 *      Changes:
 *
 * 		1.1			Unblocking Structure
 *      1.0			Final Release
 *      0.8			CS SSL2 added
 *      0.5			Function implementation
 *      0.1			Initial Structure
 */

/*
 * SPI Ports
 *
 * 	Conn	PIN		Function
 * 	----	---		--------
 * 	CN1		27		SSL2-C
 * 	CN1		28		MOSI-C
 * 	CN1		29		MISO-C
 * 	CN1		30		RSPCK-C
 */

/* -----------------------------------------------------------------------------
 * 						Configuration
 * ---------------------------------------------------------------------------*/

// Max. time to wait for buffer
#define FALSE	0
#define TRUE	1


/* -----------------------------------------------------------------------------
 * 						Includes
 * ---------------------------------------------------------------------------*/

#include "spi.h"
#include "iodefine.h"
#include "typedefine.h"

/* -----------------------------------------------------------------------------
 * 						Private Variables
 * ---------------------------------------------------------------------------*/
enum spi_states
{
	SLEEP, READ, SEND
};

static struct {
	uint8_t				finished;
	enum spi_states		state;
	uint16_t 			data;
} spi_config;




/* -----------------------------------------------------------------------------
 * 						Public Functions
 * ---------------------------------------------------------------------------*/

/* Initialize SPI Bus */
void spi_initialize ( void )
{
	// Disable Power Saving for SPI
	SYSTEM.MSTPCRB.BIT.MSTPB17 = 0;

	// Disable SPI
	RSPI0.SPCR.BIT.SPE = 0x00;

	// Set RSPI control register (SPCR)
	RSPI0.SPCR.BIT.SPMS = 0x00;		// four-wire mode
	RSPI0.SPCR.BIT.TXMD = 0x00;		// full-duplex
	RSPI0.SPCR.BIT.MODFEN = 0x01;	// fault error detection disabled
	RSPI0.SPCR.BIT.MSTR = 0x01;		// master mode
	RSPI0.SPCR.BIT.SPEIE = 0x00;	// ERROR: disable generation of RSPI IRQs
	RSPI0.SPCR.BIT.SPTIE = 0x00;	// TX: disable IRQ generation
	RSPI0.SPCR.BIT.SPRIE = 0x00;	// RX: disable IRQ generation

	// Set Pin Control on SPPCR Register
	RSPI0.SPPCR.BIT.SPLP = 0;		// no RSPI Loopback
	RSPI0.SPPCR.BIT.SPLP2 = 0;		// no RSPI Loopback
	RSPI0.SPPCR.BIT.MOIFV = 0;		// MOSI Idle Fixed Value
	RSPI0.SPPCR.BIT.MOIFE = 0;		// MOSI Idle Value Fixing disabled

	// CS/SSL Configuration
	RSPI0.SSLP.BIT.SSLP2 = 0x00;	// CS Line 1 active = 0

	// Set Bit Rate on SPBR Register
	RSPI0.SPBR = 0x05;				// SPBR = 5

	// Set Data Control on SPDCR Register (# of frames to be used)
	RSPI0.SPDCR.BIT.SPFC = 0x00;	// 0 = 1 Frame, 1 = 2 Frames
	RSPI0.SPDCR.BIT.SLSEL = 0x00;	// SSL PINs are OUTPUTS

	// Set Clock Delay Register SPCKD ( RSPCK delay )
	RSPI0.SPCKD.BIT.SCKDL = 0x00;	// 1 RSPCK

	// Set RSPI slave select negation delay register (SSLND)
	RSPI0.SSLND.BIT.SLNDL = 0x00;	// 1 RSPCK

	// Set RSPI next-access delay register (SPND) - next access delay
	RSPI0.SPND.BIT.SPNDL = 0x00;	// 1 RSPCK + 2 PCLK

	// Set RSPI control register 2 (SPCR2)
	RSPI0.SPCR2.BIT.SPPE = 0x00;	// no parity bit
	RSPI0.SPCR2.BIT.SPOE = 0x00;	// even parity bit
	RSPI0.SPCR2.BIT.SPIIE = 0x00;	// RSPI Idle Interrupt disabled
	RSPI0.SPCR2.BIT.PTE = 0x00;		// Selftest disabled

	RSPI0.SPCMD0.BIT.SSLA = 0x02;	// use SSL2 PIN
	RSPI0.SPCMD0.BIT.SSLKP = 0x00;	// SSL signal level

	RSPI0.SPCMD0.BIT.SCKDEN = 0x00;	// RSPCK delay enable
	RSPI0.SPCMD0.BIT.SLNDEN = 0x01;	// SSL negotation delay enable
	RSPI0.SPCMD0.BIT.SPNDEN = 0x00;	// next-access delay enable
	RSPI0.SPCMD0.BIT.LSBF = 0x00;	// MSB first
	RSPI0.SPCMD0.BIT.SPB = 0x0F;	// data length
	RSPI0.SPCMD0.BIT.BRDV = 0x03;	// transfer bit rate	Div. ... 500kbit/s
	RSPI0.SPCMD0.BIT.CPHA = 0x01;	// clock phase
	RSPI0.SPCMD0.BIT.CPOL = 0x01;	// clock polarity

	//Select Configuration
	RSPI0.SPSCR.BYTE = 0x00;		// SPI Sequence 0->0

	// Set Interrupt Controller
	/* no Interrupts needed */

	// Set DTC
	/* no Interrupts needed */

	// Set In/Out-put Ports
	PORTD.DDR.BIT.B0 = 1;			// RSPCK-C as OUTPUT
	PORTD.DDR.BIT.B1 = 0;			// MISO-C as INPUT
	PORTD.ICR.BIT.B1 = 1;			// MISO-C enable input buffer
	PORTD.DDR.BIT.B2 = 1;			// MOSI-C as OUTPUT
	PORTE.DDR.BIT.B0 = 1;			// SSL2-C as OUTPUT
	IOPORT.PFHSPI.BYTE = 0x02;		/* Use:
										MISO-C = PD1
										MOSI-C = PD2
										RSPCL-C = PD0
										SSL0-C = PD6
										SSL1-C = PD7
										SSL2-C = PE0
										SSL3-C = PE1 */
	IOPORT.PFGSPI.BYTE = 0x4E;		// enable MISO, MOSI, RSPCL, SSL2-C

	// Enable SPI
	RSPI0.SPCR.BIT.SPE = 0x01;		// Enable RSPI function

	// Read RSPI control Register (SPCR)
	unsigned short tmp_val = RSPI0.SPDR.WORD.H;

	// Init Update
	spi_config.finished = 0;
	spi_config.data = 0;
	spi_config.state = SLEEP;

	// END
}


/* Update Function */
void spi_update ( void )
{
	switch (spi_config.state)
	{
		case SEND:
			if (RSPI0.SPSR.BIT.SPTEF == 1)
			{
				RSPI0.SPDR.WORD.H = spi_config.data;
			
				if ( (spi_config.data & 0x8000) == 0)	// SPI READ not BIT set?
				{
					spi_config.data = 0;
					spi_config.finished = TRUE;
				}
				else {
					spi_config.state = READ;
				}
			}
			break;

		case READ:
			if (RSPI0.SPSR.BIT.SPRF == 1)
			{
				spi_config.data = (uint8_t) RSPI0.SPDR.WORD.H;
				spi_config.finished = TRUE;
				spi_config.state = SLEEP;
			}
			break;

		case SLEEP:
			/* do noting */
			break;

		default:
			break;

	}
}


/* Receive Data */
void spi_read ( uint8_t address )
{
	if (spi_config.finished == TRUE )
	{
		spi_config.finished = FALSE;
		spi_config.data = ( 0x8000 | (address<<8) );
		spi_config.state = SEND;
	}
}


/* Transmit Data */
void spi_send ( uint16_t data )
{
	if (spi_config.finished == TRUE )
	{
		spi_config.finished = FALSE;
		spi_config.data = data;
		spi_config.state = SEND;
	}
}

uint8_t spi_finished ( void )
{
	return spi_config.finished;
}

/* return SPI receive data */
uint8_t spi_data ( void )
{
	return (uint8_t) spi_config.data;
}
