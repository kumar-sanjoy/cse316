#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/delay.h>
#include "i2c.h"
#include "lcd.h"

#define BAUD 9600
// Calculation for Double Speed Mode (U2X = 1): F_CPU / (8 * BAUD) - 1
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// ==================================================
// Pin map
// ==================================================
// UART (BT module):  PD0 = RXD, PD1 = TXD
// I2C  (LCD backpack): PC0 = SCL, PC1 = SDA
// LED (light button):  PB0
//
// TB6612 #1 - LEFT side (Motor A = front-left, Motor B = rear-left)
//   AIN1 = PA0, AIN2 = PA1, PWMA = OC1A (PD5)
//   BIN1 = PA2, BIN2 = PA3, PWMB = OC1B (PD4)
//
// TB6612 #2 - RIGHT side (Motor C = front-right, Motor D = rear-right)
//   AIN1 = PA4, AIN2 = PA5, PWMA = OC0  (PB3)
//   BIN1 = PA6, BIN2 = PA7, PWMB = OC2  (PD7)
//
// STBY of BOTH TB6612 boards tied together in hardware to one pin: PB1

#define AIN1 PA0
#define AIN2 PA1
#define BIN1 PA2
#define BIN2 PA3
#define CIN1 PA4
#define CIN2 PA5
#define DIN1 PA6
#define DIN2 PA7

#define STBY_PIN PB1
#define LED_PIN  PB0

#define MOVE_SPEED 200
#define TURN_SPEED 200

// The app's light button sends its code twice per tap (press + release).
// After toggling, briefly ignore/discard further bytes so the second
// send from the same tap doesn't flip the LED right back.
#define LED_DEBOUNCE_MS 250

// ==================================================
// UART
// ==================================================

void uart_init(unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    UCSRA |= (1 << U2X);
    UCSRB = (1 << TXEN) | (1 << RXEN);
    UCSRC = (1 << URSEL) | (1 << UCSZ1) | (1 << UCSZ0);

    // Flush internal hardware RX buffer
    unsigned char dummy;
    while (UCSRA & (1 << RXC)) {
        dummy = UDR;
    }
}

void uart_transmit(char data)
{
    while (!(UCSRA & (1 << UDRE)));
    UDR = data;
}

void uart_print(const char *str)
{
    while (*str)
        uart_transmit(*str++);
}

uint8_t uart_available(void)
{
    return (UCSRA & (1 << RXC));
}

char uart_receive(void)
{
    while (!(UCSRA & (1 << RXC)));
    return UDR;
}

// ==================================================
// PWM + direction pins (4 motors, 2x TB6612)
// ==================================================

void motors_init(void)
{
    // All 8 direction pins on PORTA
    DDRA = 0xFF;
    PORTA = 0x00;

    // PWM outputs: OC1A/OC1B (PORTD), OC0 (PORTB), OC2 (PORTD)
    DDRD |= (1 << PD5) | (1 << PD4) | (1 << PD7);
    DDRB |= (1 << PB3);

    // STBY + status LED
    DDRB |= (1 << STBY_PIN) | (1 << LED_PIN);
    PORTB |= (1 << STBY_PIN); // Enable both TB6612 boards

    // Timer1: 8-bit Fast PWM, non-inverting, no prescaler (Motors A/B)
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM10);
    TCCR1B = (1 << WGM12) | (1 << CS10);
    OCR1A = 0;
    OCR1B = 0;

    // Timer0: Fast PWM, non-inverting, no prescaler (Motor C)
    TCCR0 = (1 << WGM00) | (1 << WGM01) | (1 << COM01) | (1 << CS00);
    OCR0 = 0;

    // Timer2: Fast PWM, non-inverting, no prescaler (Motor D)
    TCCR2 = (1 << WGM20) | (1 << WGM21) | (1 << COM21) | (1 << CS20);
    OCR2 = 0;
}

// dir: 0 = stop, 1 = forward, 2 = reverse
void motor_A(uint8_t dir, uint8_t speed)
{
    if (dir == 1)      { PORTA |= (1 << AIN1); PORTA &= ~(1 << AIN2); }
    else if (dir == 2) { PORTA &= ~(1 << AIN1); PORTA |= (1 << AIN2); }
    else                { PORTA &= ~((1 << AIN1) | (1 << AIN2)); }
    OCR1A = speed;
}

void motor_B(uint8_t dir, uint8_t speed)
{
    if (dir == 1)      { PORTA |= (1 << BIN1); PORTA &= ~(1 << BIN2); }
    else if (dir == 2) { PORTA &= ~(1 << BIN1); PORTA |= (1 << BIN2); }
    else                { PORTA &= ~((1 << BIN1) | (1 << BIN2)); }
    OCR1B = speed;
}

void motor_C(uint8_t dir, uint8_t speed)
{
    if (dir == 1)      { PORTA |= (1 << CIN1); PORTA &= ~(1 << CIN2); }
    else if (dir == 2) { PORTA &= ~(1 << CIN1); PORTA |= (1 << CIN2); }
    else                { PORTA &= ~((1 << CIN1) | (1 << CIN2)); }
    OCR0 = speed;
}

void motor_D(uint8_t dir, uint8_t speed)
{
    if (dir == 1)      { PORTA |= (1 << DIN1); PORTA &= ~(1 << DIN2); }
    else if (dir == 2) { PORTA &= ~(1 << DIN1); PORTA |= (1 << DIN2); }
    else                { PORTA &= ~((1 << DIN1) | (1 << DIN2)); }
    OCR2 = speed;
}

void set_left(uint8_t dir, uint8_t speed)
{
    motor_A(dir, speed);
    motor_B(dir, speed);
}

void set_right(uint8_t dir, uint8_t speed)
{
    motor_C(dir, speed);
    motor_D(dir, speed);
}

void car_forward(void)  { set_left(1, MOVE_SPEED); set_right(1, MOVE_SPEED); }
void car_backward(void) { set_left(2, MOVE_SPEED); set_right(2, MOVE_SPEED); }
void car_left(void)     { set_left(2, TURN_SPEED); set_right(1, TURN_SPEED); }
void car_right(void)    { set_left(1, TURN_SPEED); set_right(2, TURN_SPEED); }
void car_stop(void)     { set_left(0, 0); set_right(0, 0); }

// ==================================================
// LED (PB0)
// ==================================================

void led_set(uint8_t on)
{
    if (on)
        PORTB |= (1 << LED_PIN);
    else
        PORTB &= ~(1 << LED_PIN);
}

void led_debounce_drain(void)
{
    _delay_ms(LED_DEBOUNCE_MS);
    while (uart_available())
    {
        uart_receive();
    }
}

// ==================================================
// LCD status display
// ==================================================

void show_status(const char *label)
{
    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("BT:");
    lcd_set_cursor(1, 0);
    lcd_print(label);
}

// Shown for any byte without a known mapping
void show_unknown(char c)
{
    char code_buf[5];
    itoa((uint8_t)c, code_buf, 10);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_print("Unknown code:");
    lcd_set_cursor(1, 0);
    lcd_print(code_buf);

    if (c >= 32 && c <= 126)
    {
        char ch_buf[4] = {' ', '\'', c, '\0'};
        lcd_print(ch_buf);
        lcd_print("'");
    }

    uart_print("Unknown byte code=");
    uart_print(code_buf);
    uart_print("\r\n");
}

// ==================================================
// Main
// ==================================================

int main(void)
{
    uart_init(MYUBRR);
    lcd_init();
    motors_init();
    led_set(0);
    uint8_t led_on = 0;

    lcd_set_cursor(0, 0);
    lcd_print("BT Ready");
    uart_print("System Ready\r\n");

    while (1)
    {
        if (uart_available())
        {
            char c = uart_receive();

            switch (c)
            {
                case 'F': car_forward();  show_status("Forward");  break;
                case 'B': car_backward(); show_status("Backward"); break;
                case 'L': car_left();     show_status("Left");     break;
                case 'R': car_right();    show_status("Right");    break;
                case 'S': car_stop();     show_status("Stop");     break;

                case 'X':
                    led_on = !led_on;
                    led_set(led_on);
                    show_status(led_on ? "LED ON" : "LED OFF");
                    led_debounce_drain();
                    break;

                case 'Y':
                    show_status("Horn");
                    break;

                default:
                    show_unknown(c);
                    break;
            }
        }
    }
}
