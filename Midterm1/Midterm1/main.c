//Midterm1

#define F_CPU 16000000UL
#define BAUD 115200
#define FOSC 16000000
#define UBRR_VALUE (FOSC/8/BAUD -1)  //USART Baud rate equation 
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdint.h>

volatile uint8_t ADCvalue;             //Global variable, set to volatile if used with ISR
//AT array
volatile unsigned char ADCtemp[5];
volatile unsigned char CWMODE[] = "AT+CWMODE=3\r\n"; //Mode 3 for wifi mode
volatile unsigned char WIFI[] =   "AT+CWJAP=\"JIJJG\", \"xxxxxx\"\r\n"; //Connect to wifi with SSID and password
volatile unsigned char ENABLE[] = "AT+CIPMUX=0\r\n"; //enable single connection
volatile unsigned char CIPSTART[] = "AT+CIPSTART=\"TCP\",\"184.106.153.149\",80\r\n"; //Transmission control protocol, remote IP address, port 80
volatile unsigned char CIPSEND[] =  "AT+CIPSEND=45\r\n"; //45 is the length of the data to be sent
volatile unsigned char SEND_DATA[] = "GET /update?key=ML87AXCZLB8AGULF&field1="; //Write API key: ML87AXCZLB8AGULF from thingspeak
volatile unsigned char PAUSE_DELAY[] =    "\r\n\r\n";  //pause delay

//prototypes
void usart_init (void);  //usart function 
void send_ATdata (volatile unsigned char c[]);  //sends the arrays to this function

int main(void)
{
	//ADC intializations
	ADMUX = 0;              //use ADC0
	ADMUX |= (1 << REFS0);  //use AVcc as the reference
	ADMUX |= (1 << ADLAR);  //Right adjust for 8 bit resolution
	ADCSRA |= (1 << ADATE); //Set ADC Auto Trigger Enable
	ADCSRB = 0;             //0 for free running mode
	ADCSRA |= (1 << ADEN);  //Enable the ADC
	ADCSRA |= (1 << ADIE);  //Enable Interrupts
	ADCSRA |= (1 << ADSC);  //Start the ADC conversion
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);  //set prescaler to 128
	
	usart_init();      //usart function call 
	
	_delay_ms(1000);
	send_ATdata(CWMODE);
	_delay_ms(1000);
	send_ATdata(WIFI);
	_delay_ms(2000);
	send_ATdata(ENABLE);

	//while always
	while(1)
	{
		_delay_ms(1000);  //delay function
		send_ATdata(CIPSTART); //thingspeak connection in port 80
		_delay_ms(1000);  //delay function
		send_ATdata(CIPSEND); //data length  
		_delay_ms(1000);  //delay function
		send_ATdata(SEND_DATA); //api key 
		send_ATdata(ADCtemp);  //sends the temperature data from sensor 
		send_ATdata(PAUSE_DELAY);  //delay for display output
	}
}

//interrupt subroutine 
ISR(ADC_vect)
{
	volatile unsigned int j=0;
	char temp[5];

	ADCvalue = (ADCH<<1);  //shifts ADCH by 1 to the left
	itoa(ADCvalue, temp, 10);
	
	while(j<5) 
	{
		ADCtemp[j] = temp[j]; //set temp as ADCtemp 
		j++;  //increment the j temp
	}
}

void usart_init(void)  //usart initialization 
{
		//set baud rate
		UBRR0H = ((UBRR_VALUE) >> 8); //shift by 8 bits
        UBRR0L = UBRR_VALUE; //set UBRRO low to the UBRR_VALUE

		UCSR0A |= (1 << U2X0); //USART transmission speed
		UCSR0B |= (1 << TXEN0); //enable transmission and reception
		UCSR0C |= (1 << UCSZ01) | (1 << UCSZ00); //set frame to 8 data bits, no parity, 1 stop bit
		
		sei();   //enable global interrupt 
}

void send_ATdata(volatile unsigned char c[]) 
{
	//counter variable initializations
	volatile unsigned int i=0;
	volatile unsigned int j=0;
	
	j = 0; //initialize counter j to 0
	
	//while still not at the end
	while (c[j] != 0x00) 
	{ 
		j++;   //increment j counter 
	}
	
	//while i is less than j counter
	while (i<j)
	 {
		 //wait whike prevoius byte is completed
		while(!(UCSR0A & (1 << UDRE0)));  //buffer is empty and can be written to when UDREO is 1
		UDR0 = c[i]; //transmit data 
		i++;  //increment i counter 
	 }
}
