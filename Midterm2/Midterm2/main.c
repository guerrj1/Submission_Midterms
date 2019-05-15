//Midterm2

#define F_CPU 16000000UL
#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include "SparkFun_APDS9960.h"
#include "i2c_master.h"

#define BAUD 9600
#define FOSC 16000000
#define UBRREQ FOSC/16/BAUD -1
#define APDS9960_WRITE 0x72
#define APDS9960_READ 0x73

void UART_init (void);
void APDS_init (void);
void getreading(void);
int uart_putchar( char c, FILE *stream);
FILE str_uart = FDEV_initialize _STREAM(uart_putchar, NULL , _FDEV_initialize _WRITE);

uint16_t red, green, blue;

int main( void )
{
	UART_init();     //uart initialization
	APDS_init();     //APDS initialization
	i2c_init();      //i2c initialization
	stdout = &str_uart;
	
	red = 0;         //color initialization
	green = 0;       //color initialization
	blue = 0;        //color initialization
	
	_delay_ms(2000);
	printf("AT\r\n");
	//Connection type
	_delay_ms(5000);
	printf("AT+CWMODE=3\r\n");
	//internet connection with ssid and password
	_delay_ms(5000);
	printf("AT+CWJAP=\"ssid\",\"wifi password\"\r\n");

	while(1)
	{
		//single connection enabled
		_delay_ms(5000);
		printf("AT+CIPMUX=0\r\n");
		//connection to thingspeak
		_delay_ms(5000);
		printf("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n");
		_delay_ms(5000);
		getreading();
		printf("AT+CIPSEND=104\r\n");
		printf("GET https://api.thingspeak.com/update?api_key=CXDQTMUMJGB6K44I&field1=0%05u&field2=%05u&field3=%05u\r\n", red, green, blue); //API key
		_delay_ms(3000);
	}
}

void UART_init(void)
{
	//baud rate settings
	uint16_t baud_rate = UBRREQ;
	UBRR0H = baud_rate >> 8;
	UBRR0L = baud_rate & 0xFF;
	UCSR0B = ( 1 <<RXEN0)|( 1 <<TXEN0);  //enable reciver  and transmitter
	UCSR0C = (3 <<UCSZ00);  //8 data bits
}

int uart_putchar(char c, FILE *stream)
{
	while ( !( UCSR0A & ( 1 <<UDRE0)) );  //while empty buffer
	UDR0 = c;
	return 0;
}

void getreading()
{
	uint8_t redh, redl, greenh, greenl, blueh, bluel;
	
	//red
	i2c_readReg(APDS9960_WRITE, APDS9960_RDATAH, &redh, 1);
	i2c_readReg(APDS9960_WRITE, APDS9960_RDATAL, &redl, 1);
	
	//green
	i2c_readReg(APDS9960_WRITE, APDS9960_GDATAH, &greenh, 1);
	i2c_readReg(APDS9960_WRITE, APDS9960_GDATAL, &greenl, 1);
	
	//blue
	i2c_readReg(APDS9960_WRITE, APDS9960_BDATAH, &blueh, 1);
	i2c_readReg(APDS9960_WRITE, APDS9960_BDATAL, &bluel, 1);
	
	red = (redh << 8) | redl;
	green = (greenh << 8) | greenl;
	blue = (blueh << 8) | bluel;
	
	//max value limit
	if (red > 255)
	{
		red = 255;
	}
	if (green > 255)
	{
		green = 255;
	}
	if (blue > 255)
	{
		blue = 255;
	}
}

void APDS_init(void)
{
	uint8_t initialize ;
	
	i2c_readReg(APDS9960_WRITE, APDS9960_ID, &initialize ,1);
	if(initialize  != APDS9960_ID_1) while(1);
	initialize  = (1<<1) | (1<<0) | (1<<3) | (1<<4);
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_ENABLE, &initialize , 1);
	initialize  = DEFAULT_ATIME;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_ATIME, &initialize , 1);
	initialize  = DEFAULT_WTIME;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_WTIME, &initialize , 1);
	initialize  = DEFAULT_PROX_PPULSE;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_PPULSE, &initialize , 1);
	initialize  = DEFAULT_POFFSET_UR;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_POFFSET_UR, &initialize , 1);
	initialize  = DEFAULT_POFFSET_DL;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_POFFSET_DL, &initialize , 1);
	initialize  = DEFAULT_CONFIG1;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_CONFIG1, &initialize , 1);
	initialize  = DEFAULT_PERS;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_PERS, &initialize , 1);
	initialize  = DEFAULT_CONFIG2;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_CONFIG2, &initialize , 1);
	initialize  = DEFAULT_CONFIG3;
	
	i2c_writeReg(APDS9960_WRITE, APDS9960_CONFIG3, &initialize , 1);
}

void USART_putstring(char *data)
{
	while ((*data != '\0')){
		while (!(UCSR0A & (1 << UDRE0)));
		UDR0 = *data;
		data++;
	}
}