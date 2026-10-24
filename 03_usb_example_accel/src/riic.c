#include "iodefine.h"
#include "riic.h"

#define RIIC_TIMEOUT (100000UL)

static volatile uint8_t dummy;

// Wait while condition is true. Return -1 on timeout.
#define WAIT_WHILE(cond)                               \
    do {                                               \
        unsigned long t = RIIC_TIMEOUT;                \
        while (cond) {                                 \
            if (--t == 0) {                            \
                return -1;                             \
            }                                          \
        }                                              \
    } while (0)

static void stop_condition(void) {
    unsigned long t = RIIC_TIMEOUT;

    RIIC0.ICSR2.BIT.STOP = 0;
    RIIC0.ICCR2.BIT.SP = 1;
    while (RIIC0.ICSR2.BIT.STOP == 0 && --t) {
        ;
    }
    RIIC0.ICSR2.BIT.NACKF = 0;
    RIIC0.ICSR2.BIT.STOP = 0;
}

static void bus_delay(void) {
    for (int i = 0; i < 500; i++) { // ~5 us
        dummy++;
    }
}

// Slave can hold SDA low if MCU was reset in the middle of transfer
// (reset button, debugger): slave is not reset together with MCU.
// Clock it out with up to 9 SCL pulses using pins as GPIO, then generate STOP.
// Lines are driven as open drain: output 0 or input (pulled up on board).
static void bus_recover(void) {
    PORT1.DR.BIT.B2 = 0;
    PORT1.DR.BIT.B3 = 0;
    PORT1.DDR.BIT.B2 = 0; // release SCL
    PORT1.DDR.BIT.B3 = 0; // release SDA
    bus_delay();

    for (int i = 0; i < 9 && PORT1.PORT.BIT.B3 == 0; i++) {
        PORT1.DDR.BIT.B2 = 1; // SCL low
        bus_delay();
        PORT1.DDR.BIT.B2 = 0; // SCL high
        bus_delay();
    }

    // STOP: SDA goes low to high while SCL is high
    PORT1.DDR.BIT.B2 = 1;
    bus_delay();
    PORT1.DDR.BIT.B3 = 1;
    bus_delay();
    PORT1.DDR.BIT.B2 = 0;
    bus_delay();
    PORT1.DDR.BIT.B3 = 0;
    bus_delay();
}

void riic_init(void) {
    SYSTEM.MSTPCRB.BIT.MSTPB21 = 0; // enable clocks to RIIC0

    PORT1.ICR.BIT.B2 = 1; // P12 - SCL0
    PORT1.ICR.BIT.B3 = 1; // P13 - SDA0

    RIIC0.ICCR1.BIT.ICE = 0; // pins are GPIO while RIIC0 is disabled
    bus_recover();

    RIIC0.ICCR1.BIT.ICE = 0;    // RIIC0 disabled
    RIIC0.ICCR1.BIT.IICRST = 1; // RIIC0 internal reset
    for (int i = 0; i < 1000; i++) {
        dummy++;
    }
    RIIC0.ICCR1.BIT.IICRST = 0; // cancel internal reset

    RIIC0.ICSER.BIT.SAR0E = 0; // slave address 0 disabled

    // PCLK = 48MHz. PCLK / 8, ~100 kbps
    RIIC0.ICMR1.BIT.CKS = 3;
    RIIC0.ICBRL.BYTE = 0xFD;
    RIIC0.ICBRH.BYTE = 0xF8;

    RIIC0.ICMR3.BIT.ACKWP = 1; // enable access to ACKBT bit

    RIIC0.ICFER.BIT.TMOE = 0; // timeout function disabled
    RIIC0.ICFER.BIT.MALE = 1; // master arbitration lost detection enabled

    RIIC0.ICCR1.BIT.ICE = 1; // RIIC0 enabled
}

static int master_write(uint8_t addr, const uint8_t *data, size_t len) {
    WAIT_WHILE(RIIC0.ICCR2.BIT.BBSY == 1); // bus is busy

    RIIC0.ICCR2.BIT.ST = 1; // start condition

    WAIT_WHILE(RIIC0.ICSR2.BIT.TDRE == 0);
    RIIC0.ICDRT = addr << 1;

    while (len > 0) {
        unsigned long t = RIIC_TIMEOUT;

        while (RIIC0.ICSR2.BIT.TDRE == 0 && RIIC0.ICSR2.BIT.NACKF == 0 && --t) {
            ;
        }
        if (RIIC0.ICSR2.BIT.NACKF || t == 0) {
            stop_condition();
            return -1;
        }
        RIIC0.ICDRT = *data++;
        len--;
    }

    {
        unsigned long t = RIIC_TIMEOUT;

        while (RIIC0.ICSR2.BIT.TEND == 0 && --t) { // all bytes are transmitted
            ;
        }
    }

    int res = RIIC0.ICSR2.BIT.NACKF ? -1 : 0;

    stop_condition();
    return res;
}

static int master_read(uint8_t addr, uint8_t *data, size_t len) {
    if (len == 0) {
        return 0;
    }

    WAIT_WHILE(RIIC0.ICCR2.BIT.BBSY == 1); // bus is busy

    RIIC0.ICCR2.BIT.ST = 1; // start condition

    WAIT_WHILE(RIIC0.ICSR2.BIT.TDRE == 0);
    RIIC0.ICDRT = (addr << 1) | 0x01;

    WAIT_WHILE(RIIC0.ICSR2.BIT.RDRF == 0); // address transmitted, ACK checked

    if (RIIC0.ICSR2.BIT.NACKF) {
        RIIC0.ICSR2.BIT.STOP = 0;
        RIIC0.ICCR2.BIT.SP = 1;
        dummy = RIIC0.ICDRR;
        stop_condition();
        return -1;
    }

    if (len > 1) {
        dummy = RIIC0.ICDRR; // dummy read starts receiving

        WAIT_WHILE(RIIC0.ICSR2.BIT.RDRF == 0);
        while (len > 2) {
            len--;
            *data++ = RIIC0.ICDRR;
            WAIT_WHILE(RIIC0.ICSR2.BIT.RDRF == 0);
        }

        RIIC0.ICMR3.BIT.WAIT = 1; // final byte - 1
        RIIC0.ICMR3.BIT.ACKBT = 1; // NACK on last byte
        *data++ = RIIC0.ICDRR;
    } else {
        RIIC0.ICMR3.BIT.WAIT = 1;
        RIIC0.ICMR3.BIT.ACKBT = 1;
        dummy = RIIC0.ICDRR;
    }

    WAIT_WHILE(RIIC0.ICSR2.BIT.RDRF == 0);

    RIIC0.ICSR2.BIT.STOP = 0; // read final byte and issue stop condition
    RIIC0.ICCR2.BIT.SP = 1;
    *data = RIIC0.ICDRR;
    RIIC0.ICMR3.BIT.WAIT = 0;

    WAIT_WHILE(RIIC0.ICSR2.BIT.STOP == 0);
    RIIC0.ICSR2.BIT.NACKF = 0;
    RIIC0.ICSR2.BIT.STOP = 0;

    return 0;
}

// On error peripheral and bus can be left in any state (busy bus, WAIT bit set,
// slave holding SDA). Reinitialize everything, so next transfer can succeed.
int riic_master_write(uint8_t addr, const uint8_t *data, size_t len) {
    int res = master_write(addr, data, len);

    if (res != 0) {
        riic_init();
    }
    return res;
}

int riic_master_read(uint8_t addr, uint8_t *data, size_t len) {
    int res = master_read(addr, data, len);

    if (res != 0) {
        riic_init();
    }
    return res;
}
