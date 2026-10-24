#include "iodefine.h"
#include "uart.h"
#include <stdarg.h>
#include <stdio.h>

static volatile int dummy;

void uart_init(void) {
    SYSTEM.MSTPCRB.BIT.MSTPB29 = 0; // enable clocks to SCI2

    SCI2.SCR.BYTE = 0;
    PORT5.ICR.BIT.B2 = 1; // enable P52 input buffer
    SCI2.SMR.BYTE = 0;    // 8N1, PCLK

    IOPORT.PFFSCI.BIT.SCI2S = 1; // set P52 as Rx, P50 as Tx
    // Bit rate 115200: BRR = PCLK / (32 * B) - 1 = 48 * 10^6 / (32 * 115200) - 1 = 12
    SCI2.BRR = 12;
    // Wait for at least one bit interval
    for (int i = 0; i < 1000; i++) {
        dummy++;
    }

    // Transmitter only, polling mode. TXI2 interrupt is disabled in ICU,
    // its request flag IR(SCI2, TXI2) is polled to know when TDR is empty.
    // TE and TIE must be set by single write to generate the first request.
    IEN(SCI2, TXI2) = 0;
    IR(SCI2, TXI2) = 0;
    SCI2.SCR.BYTE = 0xA0; // TIE = 1, TE = 1
}

static void uart_putc(char c) {
    while (IR(SCI2, TXI2) == 0) {
        ;
    }
    IR(SCI2, TXI2) = 0;
    SCI2.TDR = c;
}

void uart_write(const char *s) {
    while (*s) {
        if (*s == '\n') {
            uart_putc('\r');
        }
        uart_putc(*s++);
    }
}

void uart_printf(const char *fmt, ...) {
    char buf[96];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);

    uart_write(buf);
}
