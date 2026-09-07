#include <avr/io.h>

int main(void)
{
    uint16_t adc_result;

    /********************************************************
     * STEP 1: Configure the ADC
     ********************************************************/

    /* ---------- ADMUX ---------- */

    // ADC source (channel)
    // MUX4:0 = 00000 -> ADC0 (PA0)
    // Change if using another ADC channel.

    // Reference voltage
    // REFS1:REFS0
    // 00 = External AREF
    // 01 = AVCC
    // 11 = Internal 2.56V

    // Result alignment
    // ADLAR = 0 -> Right Adjust (10-bit)
    // ADLAR = 1 -> Left Adjust (8-bit reading from ADCH)

    ADMUX =
        (0 << REFS1) |      // External reference (change if needed)
        (0 << REFS0) |
        (0 << ADLAR) |      // Right adjusted
        (0 << MUX4)  |
        (0 << MUX3)  |
        (0 << MUX2)  |
        (0 << MUX1)  |
        (0 << MUX0);         // ADC0


    /* ---------- ADCSRA ---------- */

    ADCSRA =
        (1 << ADEN)  |       // Enable ADC
        (0 << ADSC)  |       // Don't start yet
        (0 << ADATE) |       // Auto Trigger OFF
        (0 << ADIF)  |       // Flag (hardware controlled)
        (0 << ADIE)  |       // ADC Interrupt OFF

        // Prescaler
        // 111 = Divide by 128
        (1 << ADPS2) |
        (1 << ADPS1) |
        (1 << ADPS0);


    /* ---------- SFIOR ---------- */

    // Used only when ADATE = 1.
    // Select Auto Trigger Source here.
    // Not required for single conversion mode.


    while (1)
    {
        /****************************************************
         * STEP 2: Start ADC operation
         ****************************************************/

        // Start a single conversion
        ADCSRA |= (1 << ADSC);


        /****************************************************
         * STEP 3: Extract ADC result
         ****************************************************/

        // Wait until conversion finishes
        while (ADCSRA & (1 << ADSC));

        // Read LOW first, then HIGH
        adc_result = ADC;

        // OR equivalently:
        // uint8_t low  = ADCL;
        // uint8_t high = ADCH;
        // adc_result = ((uint16_t)high << 8) | low;

        // adc_result now contains a value from 0 to 1023
    }

    return 0;
}