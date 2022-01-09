/*
 * Copyright (C) 2020 Daniel Efimenko
 *     github.com/Danya0x07
 */

#include "tm1637.h"
#include "I2C.h"

// Command sets
#define CS_DATA     (1 << 6)
#define CS_DISPLAY  (2 << 6)
#define CS_ADDRESS  (3U << 6)  // 11000000

// Data command set bits
#define BIT_READKEY     1
#define BIT_NOAUTOINC   2
// #define BIT_TESTMODE 3  ???

// Display command set bits
#define BIT_DSS 3  // Display switch settings

/*
 *     A
 *    ---
 * F |   | B
 *    -G-
 * E |   | C
 *    ---
 *     D
 */
const unsigned char tm16_digits[0x10] = {
//    XGFEDCBA
    0b00111111,  // 0
    0b00000110,  // 1
    0b01011011,  // 2
    0b01001111,  // 3
    0b01100110,  // 4
    0b01101101,  // 5
    0b01111101,  // 6
    0b00000111,  // 7
    0b01111111,  // 8
    0b01101111,  // 9
    0b01110111,  // A
    0b01111100,  // b
    0b00111001,  // C
    0b01011110,  // d
    0b01111001,  // E
    0b01110001,  // F
};

static struct {
    unsigned char brightness;
    bool state;
} current = {1, 7};

unsigned char reverseBits(unsigned char number) {
   number = (number & 0x55) << 1 | (number & 0xAA) >> 1;
   number = (number & 0x33) << 2 | (number & 0xCC) >> 2;
   number = (number & 0x0F) << 4 | (number & 0xF0) >> 4;
   
   return number;
}

void SetDisplay(unsigned char brightness, bool state)
{

    Write_Command(reverseBits(CS_DISPLAY | brightness | state << BIT_DSS));
}

void tm1637DisplayDecimal(uint8_t number, bool dots)
{
    unsigned char bytes_arry[5] = {0};
    unsigned char digit;

    if (number > 99)
        number = 99;
    else if (number < -99)
        number = -99;

    bytes_arry[0] = reverseBits(CS_ADDRESS | 0);
    bytes_arry[1] = _empty;

    if (number < 0) {
        bytes_arry[2] = _dash;
        number = -number;
    } else {
        digit = number / 10;
        bytes_arry[3] = tm16_digits[digit];
        number -= digit * 10;
    }

    digit = number;
    bytes_arry[4] = tm16_digits[digit];
    
    Write_Command(reverseBits(CS_DATA));  // cursor auto increment
    I2C_Write_Bytes(bytes_arry, sizeof(bytes_arry));
}

void tm1637DisplayContent(unsigned char content[4])
{
    uint_fast8_t i;
    I2C_Start();
    I2C_Write_Byte(reverseBits(CS_DATA));  // cursor auto increment
    I2C_Stop();
    I2C_Start();
    I2C_Write_Byte(reverseBits(CS_ADDRESS | 0));
    for (i = 0; i < 4; i++) 
    {
	 I2C_Write_Byte(reverseBits(content[i]));
    }
    I2C_Stop();
}

//void tm1637SetState(bool state)
//{
//    current.state = state;
//    SetDisplay(current.brightness, current.state);
//}

//bool tm1637GetState(void)
//{
//    return current.state;
//}

void tm1637SetBrightness(unsigned char brightness)
{
    current.brightness = reverseBits(brightness & 0x07);
    SetDisplay(current.brightness, current.state);
}

//unsigned char tm1637GetBrightness(void)
//{
//    return current.brightness;
//}

unsigned char tm1637ReadKeys(void)
{
    unsigned char keys = 0;
    I2C_Start();
    I2C_Write_Byte(reverseBits(0x42));
    keys = I2C_Read_Byte();
    I2C_Stop();
    return keys;
}