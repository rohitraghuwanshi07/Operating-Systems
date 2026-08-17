// uart.c
#include "uart.h"

#define UART_BASE 0x10000000UL
#define UART_THR (*(volatile unsigned char*)(UART_BASE + 0)) // transmit
#define UART_RBR (*(volatile unsigned char*)(UART_BASE + 0)) // receive
#define UART_LSR (*(volatile unsigned char*)(UART_BASE + 5)) // line status

#define LSR_RX_READY 0x01
#define LSR_TX_IDLE  0x20

void uart_init(void) {
    // QEMU's UART is pre-initialized by default for the virt machine
}

void uart_putc(char c) {
    while (!(UART_LSR & LSR_TX_IDLE));
    UART_THR = c;
}

char uart_getc(void) {
    while (!(UART_LSR & LSR_RX_READY));
    return UART_RBR;
}

void uart_puts(const char *s) {
    while (*s) {
        if (*s == '\n') uart_putc('\r'); // CRLF for terminal
        uart_putc(*s++);
    }
}
