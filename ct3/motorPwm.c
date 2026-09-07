#include <avr/io.h>

int main(void) {
    unsigned int period, duty_cycle, high_time;
    unsigned char button;

    DDRA = 0b00; DDRB = 0xFF; // set port A for input, port B for output
    DDRD = 0b00100000;        // set pin D.5 for output (OC1A)

    // WGM11:WGM10 = 10: with WGM13-WGM12 to select timer mode 1110
    // Fast PWM, timer 1 runs from 0 to ICR1
    // COM1A1:COM1A0 = 10: clear OC1A when compare match, set OC1A when 0
    TCCR1A = 0b10000010; // compare match occurs timer = OCR1A
    TCCR1B = 0b00011001; // WGM13:WGM12=11; CS12:CS0=001: internal clock 1MHz, no prescaler

    period = 20000;      // PWM frequency = 50Hz, period = 20000us
    duty_cycle = 6;      // initial duty cycle
    ICR1 = period - 1;   // period of output PWM signal
    high_time = (period / 100) * duty_cycle; // calculate high time
    OCR1A = high_time - 1; // set high time of output PWM signal

    while (1) {
        if (button == PINA) // ignore repeated press
            continue;

        button = PINA; 
        PORTB = button; // store button press, display on port B

        if ((button & 0b11000000) == 0b11000000) // ignore all except buttons SW6 and SW7
            continue;

        if ((button & 0b10000000) == 0) // Increment duty cycle if switch SW7 is pressed
            duty_cycle = (duty_cycle < 12) ? duty_cycle + 1 : duty_cycle;

        if ((button & 0b01000000) == 0) // Decrement duty cycle if switch SW6 is pressed
            duty_cycle = (duty_cycle > 1) ? duty_cycle - 1 : duty_cycle;

        high_time = (period / 100) * duty_cycle; // calculate high time
        OCR1A = high_time - 1;                   // set high time of output signal
    }
}