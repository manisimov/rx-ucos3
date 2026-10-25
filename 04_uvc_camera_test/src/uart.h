#ifndef UART_H
#define UART_H

// Debug output on SCI2 (P50 - TxD2), 115200 8N1.
// Connected to UART-USB converter (/dev/ttyUSB0 on PC).

void uart_init(void);
void uart_write(const char *s);
void uart_printf(const char *fmt, ...);

#endif // UART_H
