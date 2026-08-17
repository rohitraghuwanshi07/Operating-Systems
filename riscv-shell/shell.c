#include "uart.h"

static int strcmp_simple(const char *a, const char *b) {
    while (*a && *a == *b) { a++; b++; }
    return *a - *b;
}

void main(void) {
    char buf[128];
    int i;

    uart_init();
    uart_puts("RISC-V Mini Shell\n");

    while (1) {
        uart_puts("> ");
        i = 0;

        char c;
        while ((c = uart_getc()) != '\r' && c != '\n') {
            if (i < 127) {
                buf[i++] = c;
                uart_putc(c); // echo
            }
        }
        buf[i] = '\0';
        uart_putc('\n');

        if (i == 0) continue;

        if (strcmp_simple(buf, "help") == 0) {
            uart_puts("Commands: help, clear, hello\n");
        } else if (strcmp_simple(buf, "hello") == 0) {
            uart_puts("Hello from bare-metal RISC-V!\n");
        } else if (strcmp_simple(buf, "clear") == 0) {
            uart_puts("\033[2J\033[H"); // ANSI clear screen
        } else {
            uart_puts("unknown command: ");
            uart_puts(buf);
            uart_putc('\n');
        }
    }
}
