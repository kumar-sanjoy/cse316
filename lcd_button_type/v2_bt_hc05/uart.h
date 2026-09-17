#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(unsigned int ubrr);
uint8_t uart_available(void);
char uart_receive(void);

#endif
