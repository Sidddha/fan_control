/* 
 * File:   eeprom.h
 * Author: Siddha
 *
 * Created on 2 ?????? 2022 ?., 19:39
 */

#ifndef EEPROM_H
#define	EEPROM_H

void EEPROM_WriteByte(unsigned char eepromAddress, unsigned char eepromData);
unsigned char EEPROM_ReadByte(unsigned char eepromAddress);
unsigned char EEPROM_Update(unsigned char eepromAddress, unsigned char eepromData);

#endif	/* EEPROM_H */

