#define F_CPU 1000000UL
#include <util/delay.h>
#include "lcd.h"

int main(void)
{
    lcd_init();

    lcd_set_cursor(0, 0);
    lcd_print("Hello World!");

    lcd_set_cursor(1, 0);
    lcd_print("System Ready");

    while (1)
    {
        // Static display loop
    }
}