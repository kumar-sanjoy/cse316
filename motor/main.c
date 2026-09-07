#ifndef F_CPU
#define F_CPU 1000000UL
#endif

#include <avr/io.h>

#define BAUD 9600
#define MYUBRR ((F_CPU / (8UL * BAUD)) - 1)

// ==================================================
// Motor A
// ==================================================

#define AIN1 PC0
#define AIN2 PC1

// ==================================================
// Motor B
// ==================================================

#define BIN1 PC2
#define BIN2 PC3

// TB6612 Standby
#define STBY PC4


// ==================================================
// UART
// ==================================================

void uart_init(unsigned int ubrr)
{
    UBRRH = (unsigned char)(ubrr >> 8);
    UBRRL = (unsigned char)ubrr;

    UCSRA = (1 << U2X);

    UCSRB = (1 << TXEN) | (1 << RXEN);

    UCSRC = (1 << URSEL) |
            (1 << UCSZ1) |
            (1 << UCSZ0);
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
// PWM Initialization
// ==================================================

void pwm_init(void)
{
    // Motor A PWM = PD5 / OC1A
    // Motor B PWM = PD4 / OC1B

    DDRD |= (1 << PD5) | (1 << PD4);

    // Direction pins
    DDRC |= (1 << AIN1) |
            (1 << AIN2) |
            (1 << BIN1) |
            (1 << BIN2) |
            (1 << STBY);

    // Enable TB6612
    PORTC |= (1 << STBY);

    // Timer1 8-bit Fast PWM
    // OC1A + OC1B non-inverting
    // No prescaler

    TCCR1A =
        (1 << COM1A1) |
        (1 << COM1B1) |
        (1 << WGM10);

    TCCR1B =
        (1 << WGM12) |
        (1 << CS10);

    OCR1A = 0;
    OCR1B = 0;
}


// ==================================================
// Motor A
// ==================================================

void motor_A(uint8_t dir, uint8_t speed)
{
    if (dir == 1)
    {
        // Forward
        PORTC |= (1 << AIN1);
        PORTC &= ~(1 << AIN2);
    }
    else if (dir == 2)
    {
        // Reverse
        PORTC &= ~(1 << AIN1);
        PORTC |= (1 << AIN2);
    }
    else
    {
        // Stop
        PORTC &= ~((1 << AIN1) | (1 << AIN2));
    }

    OCR1A = speed;
}


// ==================================================
// Motor B
// ==================================================

void motor_B(uint8_t dir, uint8_t speed)
{
    if (dir == 1)
    {
        // Forward
        PORTC |= (1 << BIN1);
        PORTC &= ~(1 << BIN2);
    }
    else if (dir == 2)
    {
        // Reverse
        PORTC &= ~(1 << BIN1);
        PORTC |= (1 << BIN2);
    }
    else
    {
        // Stop
        PORTC &= ~((1 << BIN1) | (1 << BIN2));
    }

    OCR1B = speed;
}


// ==================================================
// Both Motors
// ==================================================

void set_motors(uint8_t dir, uint8_t speed)
{
    motor_A(dir, speed);
    motor_B(dir, speed);
}


// ==================================================
// Main
// ==================================================

int main(void)
{
    uart_init(MYUBRR);
    pwm_init();

    while (1)
    {
        if (uart_available())
        {
            char cmd = uart_receive();

            switch (cmd)
            {
                case '0':
                    // STOP
                    set_motors(0, 0);
                    break;

                case '1':
                    // FORWARD - MEDIUM
                    set_motors(1, 128);
                    break;

                case '2':
                    // FORWARD - HIGH
                    set_motors(1, 255);
                    break;

                case '3':
                    // REVERSE - MEDIUM
                    set_motors(2, 128);
                    break;

                case '4':
                    // REVERSE - HIGH
                    set_motors(2, 255);
                    break;

                default:
                    break;
            }
        }
    }
}