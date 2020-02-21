//SPI Implementation
//Original code by Mike McCauley, modified by Eugene Rockey

#include <stdint.h>
#include <fcntl.h>
#include "bcm2835.h"

void bcm2835_spi_begin(void)
{
    // Set the SPI0 pins to the Alt 0 function to enable SPI0 access on them
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_ALT0); // CE1
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_ALT0); // CE0
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_ALT0); // MISO
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_ALT0); // MOSI
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_ALT0); // CLK
    
    // Set the SPI CS register to the some sensible defaults
    volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
    bcm2835_peri_write(paddr, 0); // All 0s
    
    // Clear TX and RX fifos
    bcm2835_peri_write_nb(paddr, BCM2835_SPI0_CS_CLEAR);
}

void bcm2835_spi_end(void)
{  
    // Set all the SPI0 pins back to input
    bcm2835_gpio_fsel(RPI_GPIO_P1_26, BCM2835_GPIO_FSEL_INPT); // CE1
    bcm2835_gpio_fsel(RPI_GPIO_P1_24, BCM2835_GPIO_FSEL_INPT); // CE0
    bcm2835_gpio_fsel(RPI_GPIO_P1_21, BCM2835_GPIO_FSEL_INPT); // MISO
    bcm2835_gpio_fsel(RPI_GPIO_P1_19, BCM2835_GPIO_FSEL_INPT); // MOSI
    bcm2835_gpio_fsel(RPI_GPIO_P1_23, BCM2835_GPIO_FSEL_INPT); // CLK
}


void bcm2835_spi_setBitOrder(uint8_t order)
{
    // BCM2835_SPI_BIT_ORDER_MSBFIRST is the only one suported by SPI0
}

// defaults to 0, which means a divider of 65536.
// The divisor must be a power of 2. Odd numbers
// rounded down. The maximum SPI clock rate is
// of the APB clock
void bcm2835_spi_setClockDivider(uint16_t divider)
{
    volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CLK/4;
    bcm2835_peri_write(paddr, divider);
}

void bcm2835_spi_setDataMode(uint8_t mode)
{
    volatile uint32_t* paddr = bcm2835_spi0 + BCM2835_SPI0_CS/4;
    // Mask in the CPO and CPHA bits of CS
    bcm2835_peri_set_bits(paddr, mode << 2, BCM2835_SPI0_CS_CPOL | BCM2835_SPI0_CS_CPHA);
}