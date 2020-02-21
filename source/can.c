//CAN Bus Driver Implementation for Raspberry PI
//by Eugene Rockey Copyright 2013 All Rights Reserved

#include <stdint.h>
#include <fcntl.h>
#include "bcm2835.h"
#include "can.h"
#include "uart.h"




//Send data from SPI/RPI to CAN controller
void spitocan(uint8_t data)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);	
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);	
	// Maybe wait for TXD
    while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
		delayMicroseconds(10);
	
    // Write to FIFO, no barrier
    bcm2835_peri_write_nb(fifo, data);
	
    // Wait for DONE to be set
    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
		delayMicroseconds(10);
	
	// Set TA = 0, and also set the barrier
    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
}

//Receive data from CAN controller to SPI/RPI
uint32_t cantospi(void)
{
	volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
    volatile uint32_t* fifo = bcm2835_spi0 + BCM2835_SPI0_FIFO/4;
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_CLEAR, BCM2835_SPI0_CS_CLEAR);	
	bcm2835_peri_set_bits(paddr, BCM2835_SPI0_CS_TA, BCM2835_SPI0_CS_TA);	
	// Maybe wait for TXD
    while (!(bcm2835_peri_read(paddr) & BCM2835_SPI0_CS_TXD))
		delayMicroseconds(10);
	
    // Wait for DONE to be set
    while (!(bcm2835_peri_read_nb(paddr) & BCM2835_SPI0_CS_DONE))
		delayMicroseconds(10);
	
	// Read any byte that was sent by the CAN controller
    uint32_t data = bcm2835_peri_read_nb(fifo);
	
	// Set TA = 0, and also set the barrier
    bcm2835_peri_set_bits(paddr, 0, BCM2835_SPI0_CS_TA);
	return data;
}

//intitialize CAN Controller for proper mode and communication.
void initCAN(void)
{
	
}

void CANtransmit(uint8_t data)
{
	
}

uint8_t CANreceive(void)
{
}

