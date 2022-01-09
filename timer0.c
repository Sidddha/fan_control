#include "timer0.h"
#include <pic16f628a.h>
#include <xc.h>

void setTMR0(void)
{
    OPTION_REG = 0x03; //Выбор источника тактового сигнала, настройка предделителя
    INTCON |= 0xe0; //Разрешение прерываний по переполнению TMR0
    TMR0 = 0x07;
}

