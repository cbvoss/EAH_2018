/*
 * sd.c
 *
 *Programm zum loggen von Daten
 *Programm auf die SD Karte.
 *
 *  Created on: 09.10.2016
 *      Author: franz
 */

#include "iodefine.h"
/**sd_init
 * Konfiguriet die MCU zur Nutzung der SD Karte
 */
void sd_system_init() {
	// System Clock
	SYSTEM.SCKCR.BIT.ICK = 0;               //12.288*8=98.304MHz
	SYSTEM.SCKCR.BIT.PCK = 1;               //12.288*4=49.152MHz

	//SPI

	RSPI0.SSLP.BYTE = 0;			//SSL0 Signal Polarity 0-active

	RSPI0.SPBR = 4;
	RSPI0.SPCMD0.WORD = 0xE480;		//8-bit SPI mode 0 ...

	RSPI0.SPCR2.BYTE = 0x00;	//Parity Disabled
	RSPI0.SPDCR.BYTE=0x14;		//only SSL0  output
	RSPI0.SPND.BYTE = 0x01;		//Next-Access Delay
	RSPI0.SPPCR.BYTE=0x30;		//Loopback  off  Mosi idle  1
	RSPI0.SPSCR.BYTE = 0x00;	//only Referenced SPCMD0
	RSPI0.SSLND.BYTE = 0x01;	//Negotation Delay

	RSPI0.SPCR.BYTE=0x49;		//SPI Mode > Full-Duplex 3 wire
}
void sd_raw_send(unsigned short dat) {
	while (!RSPI0.SPSR.BIT.SPTEF);	// wait if Transmit buffer is full
	RSPI0.SPDR.WORD.H = dat;
}

unsigned short sd_raw_resive() {
	while (!RSPI0.SPSR.BIT.SPRF);	// wait for SPDR has valid received data
	return RSPI0.SPDR.WORD.H;
}
unsigned char sd_init() {
	//Warten
	for(char i=0;i<10;i++){
		sd_raw_send(0xff);
	}

	sd_raw_send(0 | 0x40); //CMD0 -GO idle
	sd_raw_send(0);
	sd_raw_send(0);
	sd_raw_send(0);
	sd_raw_send(0);	//Parameter - 0
	sd_raw_send(0x95);	//CRC
	//warten auf bestätigung
	for(char i=0;i<100;i++){
		if(sd_raw_resive()==0x01) break;
		if(i==99) return 0;
	}
	for(char i=0;i<100;i++){
		sd_raw_send(55 | 0x40);//ACMD41 cmd55
		sd_raw_send(0);
		sd_raw_send(0);
		sd_raw_send(0);
		sd_raw_send(0);	//Parameter - 0
		sd_raw_send(0x01);//CRC
		sd_raw_resive();

		sd_raw_send(41 | 0x40);//ACMD41 cmd41
		sd_raw_send(0);
		sd_raw_send(0);
		sd_raw_send(0);
		sd_raw_send(0);	//Parameter - 0
		sd_raw_send(0x01);//CRC
		if(!sd_raw_resive()) return 1;
	}
	return 0;
}
unsigned char command(unsigned char comm, unsigned int para){

	sd_raw_send(comm | 0x40);
	sd_raw_send(para>>24);//Parameter
	sd_raw_send(para>>16);
	sd_raw_send(para>>8);
	sd_raw_send(para>>0);	//Parameter
	sd_raw_send(0x01);//CRC
	unsigned char ret=0xff;
	ret=sd_raw_resive();
	for(unsigned char i=0;i<100;i++){
		if(!sd_raw_resive()) return 1;
	}
	return 0;
}
//unsigned int dat_block[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
unsigned char sd_raw_read(unsigned int adrr, unsigned int* block) {

	if(command(17,adrr)){
		for(unsigned char i=0;i<255;i++){
			if(sd_raw_resive()==0xfe){
				unsigned int dat=0;
				//Daten abholen
				for(unsigned char j = 0;j<16;j++){
					dat|=(sd_raw_resive()<<24);
					dat|=(sd_raw_resive()<<16);
					dat|=(sd_raw_resive()<<8);
					dat|=(sd_raw_resive()<<0);
					*block=dat;
					block++;
				}
				sd_raw_resive();//CRC Verwerfen
				sd_raw_resive();
				return 1;
			}

		}
		return 0;
	}
	return 0;
}
unsigned char sd_raw_write(unsigned int adrr, unsigned int* block){
	if(command(24, adrr)){
		sd_raw_send(0xfe);
		for(unsigned char i=0;i<16;i++){
			sd_raw_send(*block>>24);//Parameter
			sd_raw_send(*block>>16);
			sd_raw_send(*block>>8);
			sd_raw_send(*block>>0);	//Parameter
			block++;
		}
		if((sd_raw_resive()&0x1f)==0x05){
			while(!sd_raw_resive());	//Warten bis schreiben abgeschlossen
			return 1;
		}else{
			sd_raw_resive();			//Error
		}
	}
	return 0;
}
