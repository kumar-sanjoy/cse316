#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "adc.h"

// TEPT4400 phototransistor wiring:
//   +5V --10k-- ADC0(PA0/pin40) --TEPT4400-- GND
// More light -> phototransistor conducts more -> node pulled toward GND
// (lower voltage). Darkness -> node pulled toward +5V (higher voltage).

int main(void)
{
    lcd_init();
    adc_init();

    char buf[17];

    while (1)
    {
        uint16_t adc_value = adc_read_avg(0, 32); // PA0 / pin 40, averaged to cut noise/flicker
        uint16_t mv = (uint32_t)adc_value * 5000UL / 1023UL; // 0-5V reference, in millivolts

        sprintf(buf, "Raw: %4u    ", adc_value);
        lcd_set_cursor(0, 0);
        lcd_print(buf);

        sprintf(buf, "%u.%03u V    ", mv / 1000, mv % 1000);
        lcd_set_cursor(1, 0);
        lcd_print(buf);

        _delay_ms(200);
    }
}
