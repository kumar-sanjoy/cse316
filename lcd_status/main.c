#include <util/delay.h>
#include "lcd.h"

int main(void)
{
    lcd_init();

    // Startup status sequence
    lcd_set_cursor(0, 0);
    lcd_print("Patience...");
    _delay_ms(500);
    lcd_clear();

    // lcd_set_cursor(0, 0);
    // lcd_print("I2C Working");
    // _delay_ms(1000);
    // lcd_clear();

    // // Final resting message
    // lcd_set_cursor(0, 0);
    // lcd_print("Hello World!");

    // lcd_set_cursor(1, 0);
    // lcd_print("System Ready");

    while (1)
    {
        lcd_set_cursor(0, 0);
        lcd_print("Patience...");
        lcd_set_cursor(1, 0);
        lcd_print("I2C Working");
        _delay_ms(10000);
        // Static display loop
    }
}
