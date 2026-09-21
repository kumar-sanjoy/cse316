#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "lcd.h"
#include "adc.h"

// Laser sensor characteristics:
// Laser ON (normal beam): raw > 500
// Laser BLOCKED / NO LIGHT (Goal): raw < 100 (drops toward 0)
#define THRESHOLD 100

// Minimum time allowed between consecutive goals (in ms)
#define MIN_TIME_BETWEEN_GOALS_MS 500

int main(void)
{
    // Configure PB0, PB1, PB2 as input with internal pull-up resistors (Active Low)
    DDRB &= ~((1 << PB0) | (1 << PB1) | (1 << PB2));
    PORTB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    lcd_init();
    adc_init();

    uint16_t score_a = 0;
    uint16_t score_b = 0; // fixed value for now

    uint16_t cooldown_timer = 0;

    uint8_t pb0_prev = 1;
    uint8_t pb1_prev = 1;
    uint8_t pb2_prev = 1;

    uint8_t lcd_timer = 0;
    char buf[17];

    while (1)
    {
        // 1. Read ADC raw value on ADC0 (PA0)
        uint16_t raw_adc = adc_read(0);

        // 2. Decrement time limit between goals
        if (cooldown_timer > 0)
        {
            cooldown_timer--;
        }

        // 3. Goal detection: when laser is BLOCKED (raw drops below THRESHOLD)
        if (raw_adc < THRESHOLD && cooldown_timer == 0)
        {
            score_a++;
            cooldown_timer = MIN_TIME_BETWEEN_GOALS_MS;
        }



        // 4. Read Active-Low Buttons with edge detection
        uint8_t pb0_now = (PINB & (1 << PB0)) ? 1 : 0;
        uint8_t pb1_now = (PINB & (1 << PB1)) ? 1 : 0;
        uint8_t pb2_now = (PINB & (1 << PB2)) ? 1 : 0;

        // Button 0 (PB0): Increment
        if (pb0_prev == 1 && pb0_now == 0)
        {
            score_a++;
        }

        // Button 1 (PB1): Decrement
        if (pb1_prev == 1 && pb1_now == 0)
        {
            if (score_a > 0)
            {
                score_a--;
            }
        }

        // Button 2 (PB2): Reset
        if (pb2_prev == 1 && pb2_now == 0)
        {
            score_a = 0;
        }

        pb0_prev = pb0_now;
        pb1_prev = pb1_now;
        pb2_prev = pb2_now;

        // 5. Update LCD every ~100 ms
        if (++lcd_timer >= 100)
        {
            lcd_timer = 0;

            snprintf(buf, sizeof(buf), "A %u- %u B       ", score_a, score_b);
            lcd_set_cursor(0, 0);
            lcd_print(buf);

            snprintf(buf, sizeof(buf), "Raw: %4u       ", raw_adc);
            lcd_set_cursor(1, 0);
            lcd_print(buf);
        }

        _delay_ms(1);
    }
}