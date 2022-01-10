/*
 * File:   main.c
 * Author: siddha
 *
 * Created on December 24, 2021, 10:39 AM
 */

// PIC16F628A Configuration Bit Settings

// 'C' source line config statements

// CONFIG
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = ON         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include <xc.h>
#include <pic16f628a.h>
#include "tm1637.h"
#include "I2C.h"
#include "eeprom.h"
#include "timer0.h"

#define _XTAL_FREQ   4000000

#define incKey 0xef
#define setKey 0x6f
#define decKey 0xaf

#define INLET_1 PORTB 0
#define INLET_2 PORTB 1
#define OUTLET_1 PORTB 2
#define OUTLET_2 PORTB 3

#define TEMP_ADDRESS 0x00
#define HUM_ADDRESS 0x01

unsigned char content[4] = {};
unsigned char buttonCount = 0;
char temp_treshold;
char hum_treshold;
char temp_count; 
char hum_count; 

unsigned char current_state;
_Bool inlet_fan_state = false;
_Bool outlet_fan_state = false;

_Bool set_mode = 0;
unsigned char current_key;
unsigned char key_holder;
volatile unsigned long counter = 2;
unsigned short debounce;
unsigned short blink;

unsigned short measure_delay;
_Bool measure_complete = 1;
char AHT10_Temperature;
char AHT10_Humidity;
unsigned char AHT10_RX_Data[6];
unsigned long AHT10_ADC_Raw;

#define AHT10_TmpHum_Cmd  0xAC
#define AHT10_Adress 0x38 << 1

static struct {
    _Bool isPressed;
    _Bool isHolded;
    _Bool released;
} key = {0, 0, 0};

void __interrupt() counterInterrupt(void)   // interrupt increases counter each ms 
{                                           
    di();
    T0IF = 0;
    counter++;
    TMR0 = (0x07);
    return;
}                                                                  

unsigned short millis(void)                 // returns current value of milliseconds program is running
{                                           
    return counter;
}

unsigned char keyHandle(void)               // Scan tm1637 keys and if some key pressed implements debounce and long press
{
    unsigned char _key;
    _key = tm1637ReadKeys();
    if ((_key != 0xff) && !key.isPressed) 
    {
        debounce = millis();
        key.isPressed = 1;
	    key_holder = _key;
    }

    if ((_key != 0xff) && (_key == key_holder) && (millis() - debounce > 250)) key.isHolded = 1;
    else key.isHolded = 0;
    
    if ((_key == 0xff) && key.isPressed) 
    {
        key.released = 1;
	    key.isPressed = 0;
	    return key_holder;
    }
    else key.released = 0;
    
    return _key;
}

void display(unsigned char param, char value)   // display literal and digit characters
{
    content[0] = param;
    content[1] = _empty;
    content[2] = _empty;
    content[3] = _empty;
    tm1637DisplayContent(content);
    tm1637DisplayDecimal(value, 0);    
}
void changeScreen()                             // refresh screen depends on display mode
{                                               // and update temp and humidity treshold in eeprom
  	if (set_mode) 
  	{
    	switch (current_state) 
		{
		case 1:
			EEPROM_Update(TEMP_ADDRESS, temp_count);
            display(_t, temp_count);
            break;
		case 2:
			EEPROM_Update(HUM_ADDRESS, hum_count);
            display(_h, hum_count);
			break;
		}
	} else 	{
		if (buttonCount % 2 == 0) 
		{
            display(_t, AHT10_Temperature);
			current_state = 1;
		} else {
            display(_h, AHT10_Humidity);
			current_state = 2;
		}
	}
}

void treshold(_Bool inc)                          // increase or decrease treshold counters
{
        if ((current_state == 1) && inc) temp_count++;
        if ((current_state == 2) && inc) hum_count++;
        if ((current_state == 1) && !inc) temp_count--;
        if ((current_state == 2) && !inc) hum_count--;
}
void incClick()                                 // do things to increase values is certain conditions
{   
    if (key.released)
    {
        buttonCount++;
        if (set_mode) treshold(1);
    }
    while (key.isHolded && set_mode)
    {
        treshold(1); 
        changeScreen();
        keyHandle();
        __delay_ms(250);
    }
}

void setClick()                                 // change state to enter the setting and blink screen for a second
{
    if (key.released)
	{
        set_mode = !set_mode;
        blink = millis();
        while (millis() - blink < 1000) 
        {                                    
            for (uint_fast8_t i = 7; i > 0; i--) 
            {
                tm1637SetBrightness(i);   
                __delay_ms(40);
            }
            for (uint_fast8_t i = 0; i < 8; i++) 
            {
                tm1637SetBrightness(i);   
                __delay_ms(40);
            }
        }
	}
}

void decClick()                                 // do things to decrease values is certain conditions
{
    if (key.released)
    {
        buttonCount--;
        if (set_mode) treshold(0);
	} 
    while (key.isHolded && set_mode)
    {
        treshold(0);
        changeScreen();
        keyHandle();
        __delay_ms(250);
    }
}

_Bool fanOn(int real, int treshold, int histeresis, _Bool fanState)     //histeresis implementation
{
    int minimum = treshold - histeresis;
    int maximum = treshold + histeresis;
    if (!fanState & (real < maximum)) return 0; 
    if (!fanState & (real > maximum)) return 1;
    if (fanState & (real > minimum)) return 1;        
    if (fanState & (real < minimum)) return 0;
    return 0;
}

void fan_control(_Bool inlet, _Bool outlet)
{
    if (inlet)
    {
        PORTB |= (1 << 0);
        PORTB |= (1 << 1);
        inlet_fan_state = 1;
    } else {
        PORTB &= ~(1 << 0);
        PORTB &= ~(1 << 1);
        inlet_fan_state = 0;
    }
    if (outlet)
    {
        PORTB |= (1 << 2);
        PORTB |= (1 << 3);
        outlet_fan_state = 1;
    } else {
        PORTB &= ~(1 << 2);
        PORTB &= ~(1 << 3);
        outlet_fan_state = 0;
    }
}

void aht10_get(void)                            // get sensors data 
{
    unsigned char buf[2] = {AHT10_Adress,   AHT10_TmpHum_Cmd};
    I2C_Start();
    I2C_Write_Bytes(buf, 2);
    I2C_Stop();
}

void main(void) 
{    
    setTMR0();                                    
    InitI2C();                                    // Initialize i2c pins
    SetDisplay(7, 1);
    temp_count = EEPROM_ReadByte(TEMP_ADDRESS);   // Restore previously set values 
    hum_count = EEPROM_ReadByte(HUM_ADDRESS);
    TRISB |= 0x0f;
    PORTB = 0x00;

	while(1) 
	{
/*        if (measure_complete) 
        {   
            aht10_get();
            measure_delay = millis();
            measure_complete = 0;
        }
        if (millis() - measure_delay > 100) 
        {
            I2C_Start();
            I2C_Write_Byte((AHT10_Adress | 1));
            for (uint_fast8_t i = 0; i < 6; i++)
            {
                AHT10_RX_Data[i] = I2C_Read_Byte();
                I2C_Send_ACK();
            }
            I2C_Stop();
            AHT10_ADC_Raw = (((unsigned long)AHT10_RX_Data[3] & 15) << 16) | ((unsigned long)AHT10_RX_Data[4] << 8) | AHT10_RX_Data[5];
            AHT10_Temperature = (unsigned long)(AHT10_ADC_Raw * 200 / 1048576) - 50;
            AHT10_ADC_Raw = ((unsigned long)AHT10_RX_Data[1] << 16) | ((unsigned long)AHT10_RX_Data[2] << 8) | (AHT10_RX_Data[3] >> 4);
            AHT10_Humidity = (unsigned long)(AHT10_ADC_Raw/1048576);
            measure_complete = 1;
        }*/
        
        current_key = keyHandle();
		switch (current_key)
		{
		case incKey:
			incClick();
				break;
		case setKey:
			setClick();
				break;
		case decKey:
			decClick();
				break;
		default:
				break;
		}
		changeScreen();
        temp_treshold = EEPROM_ReadByte(TEMP_ADDRESS);
        hum_treshold = EEPROM_ReadByte(HUM_ADDRESS);
        if (fanOn(AHT10_Temperature, temp_treshold, 1, inlet_fan_state)) 
        {
            fan_control(1, 1);  
        }
        if (fanOn(AHT10_Humidity, hum_treshold, 1, outlet_fan_state)) 
        {
            fan_control(0 ,1);
        } else {
            fan_control(0, 0);
        } 
	}
	return;
}


