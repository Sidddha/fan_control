/*
 * Copyright (C) 2020 Daniel Efimenko
 *     github.com/Danya0x07
 */

/*----------------------------------------------------------
 * A simple and cross-platform library 
 * for interfacing TM1637 four-digit seven-segment indicators.
 *---------------------------------------------------------*/

#ifndef _TM1637_H
#define _TM1637_H

#include <stdint.h>
#include <stdbool.h>
/* The segment map:
 *     A
 *    ---
 * F |   | B
 *    -G-
 * E |   | C
 *    ---
 *     D
 * The enumeration can be extended with user symbols. 
 * For the 2nd (counting from left, [12:34]) digit of the display
 * the X bit controls the dots.
 */
enum TM1637_letters {
//    _A = 0x77,
//    _B = 0x7f,
//    _C = 0x39,
//    _D = 0x3f,
//    _E = 0x79,
//    _F = 0x71,
//    _G = 0x3d,
//    _H = 0x76,
//    _J = 0x1e,
//    _L = 0x38,
//    _N = 0x37,
//    _O = 0x3f,
//    _P = 0x73,
//    _S = 0x6d,
//    _U = 0x3e,
//    _Y = 0x6e,
//    _a = 0x5f,
//    _b = 0x7c,
//    _c = 0x58,
//    _d = 0x5e,
//    _e = 0x7b,
//    _f = 0x71,
    _h = 0x74,
//    _i = 0x10,
//    _j = 0x0e,
//    _l = 0x06,
//    _n = 0x54,
//    _o = 0x5c,
//    _q = 0x67,
//    _r = 0x50,
    _t = 0x78,
//    _u = 0x1c,
//    _y = 0x6e,
    _dash = 0x40,
//    _under = 0x08,
//    _equal = 0x48,
    _empty = 0x00,
//    _degree = 0x63,
//    _dots = 0x80,

    _0 = 0x3f,
    _1 = 0x06,
    _2 = 0x5b,
    _3 = 0x4f,
    _4 = 0x66,
    _5 = 0x6d,
    _6 = 0x7d,
    _7 = 0x07,
    _8 = 0x7f,
    _9 = 0x6f,

};

/* Array for converting one-digit numbers(0..0xF) into TM16xx symbols.
 * Needed for displaying numbers with some caption (like [-00-] or [PH:23])
 * Example:
 *  int hp = 75;
 *  ...
 *  unsigned char content[4] = 
 *      {TM16_H, TM16_P | TM16_DOTS, tm16_digits[hp / 10], tm16_digits[hp % 10]};
 *  tm1637DisplayContent(content);  // The segments will look like [HP:75].
 */
extern const unsigned char tm16_digits[0x10];

void SetDisplay(unsigned char brightness, _Bool state);

/* Display some user content(unsigned char[4]) on the segments.
 * Example:
 *  const unsigned char content[4] = 
 *  	{_y, _o | _dots, _U, _r}; // yo:Ur
 *  tm1637DisplayContent(content); */
void tm1637DisplayContent(unsigned char content[4]);

/* Display a decimal number (-999..9999) on the segments
 * displaying dots(1) or not(0). */
void tm1637DisplayDecimal(uint8_t number, _Bool dots);


/* Set the display state on(1) or off(0). */
void tm1637SetState(_Bool state);

/* Get the display state (on(1) or off(0)). */
//_Bool tm1637GetState(void);

/* Set the segments brightness(0..7).
 * Side effect: if the display was turned off then it turns on. */
void tm1637SetBrightness(unsigned char brightness);

/* Get the segments brightness(0..7). */
//unsigned char tm1637GetBrightness(void);

unsigned char tm1637ReadKeys(void);

#endif  // _TM1637_H
