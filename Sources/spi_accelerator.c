/*
 * spi_accelerator.c
 *
 * 		Peripherie Module: Acceleration Sensor
 *
 *      Author: 		Andreas Chwojka
 *      Date:			2018-02-19
 *      Version:		1.2
 *
 *		Dependencies:	SPI Module
 *						Global Clock Timer
 *						fixed_update procedure for update function
 *
 *		Memory usage:	~ 12 Bytes
 *
 *		Last Changes:	- unblocking structure
*/

/* -----------------------------------------------------------------------------
 * 						Includes and common defines/enums
 * ---------------------------------------------------------------------------*/

#include "spi_accelerator.h"

// G
#define CONST_g 9.81f

// True/False for internal use
#define FALSE 0x00
#define TRUE 0x01

// Module States
enum spi_acc_module_states {
	DISABLED, SEND, READX, READY, READZ, NEXT_SENSOR
};
enum spi_acc_sensors {
	Sensor_X, Sensor_Y, Sensor_Z
};


/* -----------------------------------------------------------------------------
 * 						Configuration
 * ---------------------------------------------------------------------------*/

// Enable Axis to read
#define READ_X
#define READ_Y
//#define READ_Z

//Sensor Config DEFAULTS: 4g Range (+/- 2g), 10bit Mode
//#define FULLRES		// Full Resolution Mode -> output = 4mg/LSB * g Range
#define RANGE_8g	// +/- 4g
//#define RANGE_16g	// +/- 8g
//#define RANGE_32g	// +/- 16g



/* -----------------------------------------------------------------------------
 * 						Private Module Data
 * ---------------------------------------------------------------------------*/
static struct {

	// Configuration
	struct {
		enum spi_acc_module_states state;		// Module State
		enum spi_acc_sensors	   sensor;		// Sensor/Axis
	} config;

	// Acceleration Data
	struct {

		struct spi_acc_data values;
		struct spi_acc_data offset;

	} acceleration;

} spi_acc_data;


/* -----------------------------------------------------------------------------
 * 						Module Functions
 * ---------------------------------------------------------------------------*/

/* Initialize Accelerator Module */
void spi_acc_initialize ( void )
{
	// Init Module Structure Values
	spi_acc_data.config.state = SEND;
	spi_acc_data.config.sensor = Sensor_X;
	spi_acc_data.acceleration.values.x = 0;
	spi_acc_data.acceleration.values.y = 0;
	spi_acc_data.acceleration.values.z = 0;
	spi_acc_data.acceleration.offset.x = 0;
	spi_acc_data.acceleration.offset.y = 0;
	spi_acc_data.acceleration.offset.z = 0;

	// Data Measurement Options
	uint8_t data_format =	0x04;	// Justify MSB left

	#ifdef FULLRES
		data_format |= 0x08;	// 4 mg/LSB * g Range
	#endif

	#ifdef RANGE_32g
		data_format |= 0x03;	// Range: +/-  16g
	#elif defined RANGE_16g
		data_format |= 0x02;	// Range: +/-  8g
	#elif defined RANGE_8g
		data_format |= 0x01;	// Range: +/-  4g
	#endif

	spi_send ( (ACC_DATA_FORMAT)<<8
				| data_format);

	// Activate Measure Mode
	spi_send ( ACC_POWER_CTL<<8
					| 0x08 );	// Measure Mode
	// Set Offsets
	spi_acc_reset_offset();
}


/* Reset Offsets
 *
 * Blocking Structure!
 */
void spi_acc_reset_offset( void )
{
	#ifdef READ_X
		// Read and Store X
		spi_read(ACC_DATAX1);
		while( spi_finished() == FALSE);
		spi_acc_data.acceleration.offset.x = spi_convert_g_to_m(spi_data());
	#endif

	#ifdef READ_Y
		// Read and Store Y
		spi_read(ACC_DATAY1);
		while( spi_finished() == FALSE);
		spi_acc_data.acceleration.offset.y = spi_convert_g_to_m(spi_data());
	#endif

	#ifdef READ_Z
		// Read and Store Z
		spi_read(ACC_DATAZ1);
		while( spi_finished() == FALSE);
		spi_acc_data.acceleration.offset.z = spi_convert_g_to_m(spi_data());
	#endif
}


/* Update Acceleration Data */
void spi_acc_update ( void )
{
	switch (spi_acc_data.config.state)
	{
		case SEND:

			switch (spi_acc_data.config.sensor)
			{
				case Sensor_X:
					spi_read( ACC_DATAX1 );
					spi_acc_data.config.state = READX;
					break;

				case Sensor_Y:
					spi_read( ACC_DATAY1 );
					spi_acc_data.config.state = READY;
					break;

				case Sensor_Z:
					spi_read( ACC_DATAZ1 );
					spi_acc_data.config.state = READZ;
					break;

				default:
					break;
			}

			break;

		case READX:
			if ( spi_finished() == TRUE )
			{	spi_acc_data.acceleration.values.x
							= spi_convert_g_to_m(spi_data())
							- spi_acc_data.acceleration.offset.x;
				spi_acc_data.config.state = NEXT_SENSOR;
			}
			break;

		case READY:
			if ( spi_finished() == TRUE )
			{	spi_acc_data.acceleration.values.y
							= spi_convert_g_to_m(spi_data())
							- spi_acc_data.acceleration.offset.y;
				spi_acc_data.config.state = NEXT_SENSOR;
			}
			break;

		case READZ:
			if ( spi_finished() == TRUE )
			{	spi_acc_data.acceleration.values.z
							= spi_convert_g_to_m(spi_data())
							- spi_acc_data.acceleration.offset.z;
				spi_acc_data.config.state = NEXT_SENSOR;
			}
			break;

		// Switch to next Sensor
		case NEXT_SENSOR:

			// X -> Y
			if ( spi_acc_data.config.sensor == Sensor_X ) spi_acc_data.config.sensor = Sensor_Y;
			#ifdef READ_Y
				spi_acc_data.config.state = SEND;
				break;
			#endif

			// Y -> Z
			if ( spi_acc_data.config.sensor == Sensor_Y ) spi_acc_data.config.sensor = Sensor_Z;
			#ifdef READ_Y 
				spi_acc_data.config.state = SEND;
				break;
			#endif

			// Z -> X
			if ( spi_acc_data.config.sensor == Sensor_Z ) spi_acc_data.config.sensor = Sensor_X;
			#ifdef READ_Z 
				spi_acc_data.config.state = SEND;
			#endif

			break;

		case DISABLED:
			/* do nothing */
			break;

		default:
				break;
	}
}


/*
 * 	Get Acceleration Values
 *
 *	returns Values struct of stored Values (Name: spi_acc_data)
 *
*/
struct spi_acc_data spi_acc_get_accleration( void )
{
	return spi_acc_data.acceleration.values;
}


/* Force read Acceleration Sensor and return value
 * 0 = X Axis
 * 1 = Y Axis
 * 2 = Z Axis
 * other return 0
 * */
uint8_t spi_acc_force_read ( uint8_t sensor_id )
{
	// Convert Sensor ID to device specific ID
	switch (sensor_id)
	{
	case 0:
			sensor_id = ACC_DATAX0;
			break;
	case 1:
			sensor_id = ACC_DATAY0;
			break;
	case 2:
			sensor_id = ACC_DATAZ0;
			break;
	default	:
			return 0;
			break;
	}

	// Read and Return ID (blocked)
	spi_read( sensor_id );
	while(spi_finished == FALSE);
	return spi_data();
}

/* Set Refresh time for update function
 * in ms (Miliseconds)
 *
 * 0 (zero) disables module
*/
void spi_acc_set_active ( uint8_t active_var )
{
	if ( active_var == FALSE)
	{
		spi_acc_data.config.state = DISABLED;
	}
	else {
		spi_acc_data.config.state = SEND;
	}
}


/* Convert g-Value to m/s^2 */
static float spi_convert_g_to_m( uint8_t g_value )
{
	float conv_factor = 0,  ret_val = 0;

	#ifndef FULLRES

		#ifdef RANGE_32g
			conv_factor = 32.0f / 255;
		#elif defined RANGE_16g
			conv_factor = 16.0f / 255;
		#elif defined RANGE_8g
			conv_factor = 8.0f / 255;
		#else
			conv_factor = 4.0f / 255;
		#endif

		ret_val = conv_factor * (g_value - 127);

	#else

		// Code to convert fullres
		#ifdef RANGE_32g
			conv_factor = 0.004f * 32;
		#elif defined RANGE_16g
			conv_factor = 0.004f * 16;
		#elif defined RANGE_8g
			conv_factor = 0.004f * 8;
		#else
			conv_factor = 0.004f * 4;
		#endif
		ret_val = conv_factor * ((g_value & 0x7F)<<2);
		if ( (ret_val & 0x80) > 1 ) ret_val *= -1.0f;

	#endif

	return ret_val * CONST_g;
}
