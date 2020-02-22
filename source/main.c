// main.c - main for the CECS 525 Raspberry PI kernel
// by Eugene Rockey Copyright 2015 All Rights Reserved
// debug everything that needs debugging
// Add, remove, modify, preserve in order to fulfill project requirements.

#include <limits.h>
#include <stdint.h>
#include "uart.h"
#include "mmio.h"
#include "bcm2835.h"
#include "can.h"
#include "softfloat.h"
#include "math.h"

#define SECS 0x00
#define MINS 0x01
#define HRS	 0x02
#define DOM	 0x04
#define MONTH 0x05
#define YEAR 0x06
#define ASECS 0x07
#define CR 0x0D
#define GPUREAD	0x2000B880
#define GPUPOLL	0x2000B890
#define GPUSENDER	0x2000B894
#define GPUSTATUS	0x2000B898
#define GPUCONFIG	0x2000B89C
#define GPUWRITE	0x2000B8A0



const char MS1[] = "\r\n\nCECS-525 RPI Tiny OS";
const char MS2[] = "\r\nby Eugene Rockey Copyright 2013 All Rights Reserved";
const char MS3[] = "\r\nReady: ";
const char MS4[] = "\r\nInvalid Command Try Again...";
const char GPUDATAERROR[] = "\r\nSystem Error: Invalid GPU Data";
const char LOGONNAME[] = "eugene    ";
const char PASSWORD[] = "cecs525   ";
//PWM Data for Alarm Tone
uint32_t N[200] = {0,1,2,3,4,5,6,7,8,9,10,11,12,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,
				36,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,60,61,62,63,64,65,66,67,68,69,
				70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,84,85,86,87,88,89,90,91,92,93,94,95,96,95,94,93,92,91,90,
				89,88,87,86,85,84,84,83,82,81,80,79,78,77,76,75,74,73,72,71,70,69,68,67,66,65,64,63,62,61,60,60,59,58,57,
				56,55,54,53,52,51,50,49,48,47,46,45,44,43,42,41,40,39,38,37,36,36,35,34,33,32,31,30,29,28,27,26,25,24,23,
				   22,21,20,19,18,17,16,15,14,13,12,12,11,10,9,8,7,6,5,4,3,2,1};
char logname[10];
char pass[10];
char* buffer[1];
char alarm[1];
uint8_t ones;
uint8_t tens;
char* tbuf;
char* rbuf;
void kernel_main();             //prototypes
void enable_arm_irq();
void disable_arm_irq();
void enable_arm_fiq();
void disable_arm_fiq();
void reboot();
void enable_irq_57();
void disable_irq_57();
void testdelay();
extern int invar;               //assembly variables
extern int outvar;



//Pointers to some of the BCM2835 peripheral register bases
volatile uint32_t* bcm2835_gpio = (uint32_t*)BCM2835_GPIO_BASE;
volatile uint32_t* bcm2835_clk = (uint32_t*)BCM2835_CLOCK_BASE;
volatile uint32_t* bcm2835_pads = (uint32_t*)BCM2835_GPIO_PADS;//for later updates to program
volatile uint32_t* bcm2835_spi0 = (uint32_t*)BCM2835_SPI0_BASE;
volatile uint32_t* bcm2835_bsc0 = (uint32_t*)BCM2835_BSC0_BASE;//for later updates to program
volatile uint32_t* bcm2835_bsc1 = (uint32_t*)BCM2835_BSC1_BASE;
volatile uint32_t* bcm2835_st = (uint32_t*)BCM2835_ST_BASE;

int serial_getc();
void serial_putc(char c);
void serial_puts(const char* str);

void enable_irq_57(void)
{
	mmio_write(0x2000B214, 0x02000000); //Enables UART interrupts
}

void disable_irq_57(void)
{
	mmio_write(0x2000B220, 0x02000000); //Disables UART interrupts
}

void banner(void)
{
	uart_puts(MS1);
	serial_puts(MS2);
}

int calc_add(int l, int r); //Declares the assembly addition function
int calc_sub(int l, int r); //Declares the assembly subtraction function
long long calc_mul(int l, int r); //Declares the assembly multiplication function
int calc_div(int* rem, int l, int r); //Declares the assembly division function

int atoi__(const char* v) //Used to convert a string to an integer type
{
	int acc = 0; //Initializes accumulated value
	int s = 1; //Initializes the sign variable.
	if(*v == '-')
	{
		s = -1; //If the string number is negative, the resulting value below will be multiplied by -1
		++v; //Progress string by one element
	}
	while(*v != '\0') //While string character does not equal the end...
		acc = acc * 10 + *(v++) - '0'; //Acc is calculated as the integer value pulled from the string input
	return acc*s; //Resulting integer is multipled by the sign variable (see line 108)
}

const char* reverseStr(char* str, uint32_t len) //Returns a reverse string version of the string input
{
	uint32_t l = 0; //Initializes first variable which will be the first element of the input string
	uint32_t r = len-1; //Initializes the second variable which will be the last element in the input string.
	while(l < r) //While string character variables are not in the middle of the string...
	{
		char tmp = str[l]; //Use SWAP format (temp variable)
		str[l] = str[r]; //to swap first and last elements until middle of string is reached.
		str[r] = tmp;
		++l;
		--r;
	}
	
	return str;
}

const char* itoa__(int val, char* buf, int base) //Returns a string version of an integer value input.
{
	if(val == 0) //If the input integer is 0, return a string value of "0"
		return "0";
	
	uint32_t i = 0; //Used to iterate through the output string variable
	uint8_t n = 0; //Used to flag if a String variable as a negative.
	if(val < 0)
	{
		n = 1; //If input value is negative, the negative flag is set high.
		val = -val; //The input value is then negated
	}
	
	
	int rem = 0; //Used in implementation of calc div
	while(val != 0)
	{
		val = calc_div(&rem,val,base); //Find the modulus (remainder of div) to convert each digit in the integer into a string character
 		buf[i] = ((uint8_t) rem) + '0';
		i++;
	}
	if(n)
		buf[i++] = '-'; //If negative flag was set high, add negative sign to string
	buf[i] = '\0'; //Add NULL ending character when process is completed.
	
	
	return reverseStr(buf,i); //Return reversed string verison of created output, as its current state is a flipped version of the desired outcome.
}

const char* lltoa__(long long val, char* buf, int base) //Identical in function to itoa but converts long long types into strings.
{
	if(val == 0)
		return "0";
	
	uint32_t i = 0;
	uint8_t n = 0;
	if(val < 0)
	{
		n = 1;
		val = -val;
	}
	
	
	int rem = 0;
	while(val != 0)
	{
		val = calc_div(&rem,val,base);
		buf[i] = ((uint8_t) rem) + '0';
		i++;
	}
	if(n)
		buf[i++] = '-';
	buf[i] = '\0';
	return reverseStr(buf,i);
}

int serial_read_num(int* succeed) //Used to take user input from terminal and validates that it's a number
{
	char buf[12]; //Creates buffer to store input
	uint8_t n = 0;
	while(1)
	{
		do buf[n] = serial_getc(); while(buf[n] == '\0'); //Uses getc to accept input character-by-character via interrupts.
		if(buf[n] == '\r') //If no user input is provided...
		{
			buf[n] = '\0'; //Buffer is declared empty.
			serial_puts("\n"); //Creates a new line
			
			break;
		}
		n++; //Increments buffer element when character is accepted.
		
		if(n == 12) //If buffer is exceeded...
		{
			serial_puts("\r\nNumber too large!");
			*succeed = 0;
			return 0; //Resets buffer and returns 0 as an integer.
		}
	}
	n = 0;
	if(buf[n] == '-') //Ignore negative signs when validating if input is a number
		++n;
	
	for(; buf[n] != '\0'; ++n) //Process used to validate if input consists only of digits.
		if('0' > buf[n] || buf[n] > '9' )
		{
			serial_puts("\r\nNot a number.");
			*succeed = 0;
			return 0;
		}
	
	*succeed = 1;
	return atoi__(buf); //Sends successful inptu to atoi to convert to integer type
}

void calc(void) //Handles calculator functions. Functionally identical to the first demo code.
{
	for(;;)
	{
		serial_puts("\r\nCalculator\r\n==========\r\n\t(1) Add\r\n\t(2) Subtract\r\n\t(3) Multiply\r\n\t(4) Divide\r\n\t(0) Exit\r\nEnter #: ");
		
		char buf[20]; //Used to store calculator inputs.
		int succeed = 0;

		int val = serial_read_num(&succeed); //Used to accept user input (the function choice)
		if(!succeed) //If value is not an acceptable input...
		{
			continue; //Reset calc function.
		}
		if(val == 0) //If input is '0', exit.
			return;
		int l; //Used to store the first operand input
		do
		{
			if(!succeed)
				serial_puts("\r\nPlease enter a number.");
			serial_puts("\r\nFirst Number: ");
			l = serial_read_num(&succeed); //Used to store user input into l
		} while(!succeed);
		serial_puts(itoa__(l,buf,10));
		int r; //Used to store the second operand input
		do
		{
			if(!succeed)
				serial_puts("\r\nPlease enter a number.");
			serial_puts("\r\nSecond Number: ");
			r = serial_read_num(&succeed); //Used to store user input into r
		} while(!succeed);
		serial_puts(itoa__(r,buf,10));
		

		int rem;
		int res; //Declares division variables to use in the calc_div function.
		
		serial_puts("\r\nAnswer:");
		
		switch(val)
		{
			case 1:
			serial_puts(itoa__(calc_add(l,r),buf,10)); //Returns calc_add operation with two specified inputs. Stored into buf.
			break;
			case 2:
			serial_puts(itoa__(calc_sub(l,r),buf,10)); //Returns calc_sub operation with two specified inputs. Stored into buf.
			break;
			case 3:
			serial_puts(lltoa__(calc_mul(l,r),buf,10)); //Returns calc_mul operation with two specified inputs. Stored into buf.
			break;
			case 4: //Returns calc_div operation with two specified inputs. Stored into buf.
			if(!r)
			{
				serial_puts("\r\nCan't have a 0 as a divisor.");
				break;
			}
			res = calc_div(&rem,l,r);
			serial_puts("Result ");
			serial_puts(itoa__(res,buf,10));
			serial_puts(" Remainder ");
			serial_puts(itoa__(rem,buf,10));
			break;
			default:
			serial_puts("\r\nNot an operation.");
		}
	}
}

void RES(void)
{
	reboot();
}

void HELP(void) //Command List
{
	serial_puts("\r\n(C)alc,(H)elp,(R)eset,(S)tring");
}

void serial_puts(const char *str);
void String() //Prints a string using interrupts when called.
{
	serial_puts("\r\nHello, God. Please make this work.");
}

void command(void)
{
	serial_puts(MS3);
	uint8_t c = '\0';
	while (c == '\0') {
		c = serial_getc();
	}
	switch (c) {
		case 'C' | 'c':
			calc();
			break;
		case 'R' | 'r':
			RES();
			break;
		case 'S' | 's':
			String();
			break;
		case 'H' | 'h':
			HELP();
			break;
		default:
			serial_puts(MS4);
			HELP();
			break;
	}
}

void testdelay(void)
{
	int count = 0xFFFFF;
	while(count > 0) { count - 1; }
}

const char* iwptr = 0;
void kernel_main() //Used to initialize the TinyOS kernel
{
	iwptr = 0;
	uart_init();
	enable_irq_57();
	enable_arm_irq();
	banner();
	HELP();
	while (1) {command();}
	//while (1)
	//{
	//	uart_putc(' ');
	//	uart_putc('a');
	//	uart_putc(' ');
	//	testdelay();
	//}
}

#define BUFF_SIZE 2048

char irbuff[BUFF_SIZE];
uint32_t r_r = 0;
uint32_t r_w = 0;

int serial_getc() //Used to accept user input character by character
{
	if(r_r == r_w)
		return '\0'; //If input was empty
	 char tmp = irbuff[r_r++]; //Tmp is character accepted from the buffer, which olds interrupt inputs
	 if(r_r >= BUFF_SIZE)
		r_r = 0;
	return tmp; //Return the accepted input character
}

void uart_set_transmit_interrupt(int b); //Flag transmit interrupt as high.
int uart_is_transmit_interrupt_enabled(); //Boolean to determine if transmit interrupt is enabled.

void serial_puts(const char *str) //Used to print a string out to the terminal using interrupts.
{
	while(iwptr){}
	//while(uart_is_transmit_interrupt_enabled()){}
	iwptr = str; //Prints the contents of input string by inserting string into the iwptr buffer, which uses interrupts to print items.
	uart_set_transmit_interrupt(1); //Enable transmit interrupt to print string.
}

void serial_putc(char c) //Used to print a character out to the terminal
{
	char buf[2]; //Buffer used to store character to print
	buf[0] = c;
	buf[1] = '\0';
	serial_puts(buf); //Used serial_puts to print the character and null character
	while(uart_is_transmit_interrupt_enabled()){} //Waiting hold while transmit interrupt is enabled.
}

int uart_is_transmittable(); 
int uart_is_receivable();

void irq_handler(void)
{
	if (uart_is_receivable()) //IRQ branches here if intent is to recieve data.
	{
		uint8_t c  = uart_readc(); //Reads character from UART
		uart_putc(c); //Prints character
		
		irbuff[r_w++] = c; //Adds character to buffer
		if(r_w >= BUFF_SIZE) //If input length is larger than buffer, reset buffer.
			r_w = 0;
	}
	if (uart_is_transmittable()) //IRQ branches here if intent is to transmit data.
	{
		uart_putc(*iwptr++); //Print character by character contents of iwptr buffer
		if(*iwptr == '\0') //When end of buffer is reached
		{
			iwptr = 0; //Reset iwptr buffer
			uart_set_transmit_interrupt(0); //Disable UART transmitter interrupt.
		}
	}
}

