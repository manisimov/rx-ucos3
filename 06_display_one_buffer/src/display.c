#include "display.h"
#include "render.h"
#include "iodefine.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <os.h>

#define FRAME_PERIOD (200u) // Ticks for one frame. By now one tick = 1ms.

// MY reverse direction
#define LCD_FUNCTION_ZERO    0X28
#define LCD_FUNCTION_ONE     0X29
#define LCD_FUNCTION_TWO     0X2A
#define LCD_FUNCTION_THREE   0X2B
     // command sets
#define LCD_DISPLAY_REVERSE  0x280D
#define LCD_DISPLAY_NORMAL   0x280C

#define DISPLAY_BUFFER_SIZE (8 * 12 * 8)
static uint8_t display_buffer[DISPLAY_BUFFER_SIZE];

#define  DISPLAY_TASK_STK_SIZE                      (256u)
#define  DISPLAY_TASK_PRIO                           10u

static  OS_TCB       DisplayTaskTCB;
static  CPU_STK      DisplayTaskStk[DISPLAY_TASK_STK_SIZE];

static void task(void *p_arg);

void display_put_pixel(size_t x, size_t y) {
    if (x > 95 || y > 63)
        return;
    size_t tile_y = y / 8;
    size_t byte_y = y % 8;
    size_t column_x = x;
    size_t buffer_index = tile_y * 12 * 8 + column_x;

    display_buffer[buffer_index] |= (uint8_t)(0x01 << byte_y);
    return;
}

static void send_byte(uint8_t b) {
    PORTC.DR.BIT.B2 = 0; // chip select on

    /* ensure transmit register is empty */
    while (RSPI0.SPSR.BIT.IDLNF) ;

    /* short 16 bits */
    RSPI0.SPDR.WORD.L = 0 ;
    /* short 16 bits */
    RSPI0.SPDR.WORD.H = b;

    /* Wait for transfer to complete */
    while (RSPI0.SPSR.BIT.IDLNF) ;

    (void)RSPI0.SPDR.WORD.L;
    (void)RSPI0.SPDR.WORD.H;

    PORTC.DR.BIT.B2 = 1; // chip select off
}

static void send_command(uint8_t b) {
    PORT5.DR.BIT.B1 = 0 ; // set command
    send_byte(b);
    PORT5.DR.BIT.B1 = 1 ; // set data
}

static void send_data(uint8_t b) {
    PORT5.DR.BIT.B1 = 1 ; // set data
    send_byte(b);
    PORT5.DR.BIT.B1 = 1 ; // set data
}

void display_clear_buffer(void) {
    memset(display_buffer, 0, sizeof(display_buffer));
}

void display_init(void) {
    volatile int i = 0;

    /* Set data direction for this bit to output */
    PORTC.DDR.BIT.B3 = 1 ;   // LCD GPIO for Reset LCD
    PORT5.DDR.BIT.B1 = 1 ;   // LCD RS A0

    //StartResetLCD();
    PORTC.DR.BIT.B3 = 1;
    for (i=0;i<1000000;i++){}
    //EndResetLCD();
    PORTC.DR.BIT.B3 = 0 ;
    for (i=0;i<1000000;i++){}

    MSTP(RSPI0) = 0;

    /* Select proper bank of pins for SPI0 */
    IOPORT.PFGSPI.BIT.RSPIS = 0 ;
    /* SCK (PC.5) is active */
    IOPORT.PFGSPI.BIT.RSPCKE = 1 ;

    /* SSL3 (PC.2) is inactive (toggled as GPIO instead) */
    IOPORT.PFGSPI.BIT.SSL3E = 0 ;
    /* MOSI (PC.6) is active */
    IOPORT.PFGSPI.BIT.MOSIE = 1 ;

    /* Set up chip select pin */
    /* Make it an output */
    PORTC.DDR.BIT.B2 = 1 ;
    /* Set level to inactive */
    PORTC.DR.BIT.B2 = 1 ;

    /* MISO as an input */
    PORTC.ICR.BIT.B7 = 1 ;
    PORTC.DDR.BIT.B7 = 0 ;

    /* MOSI as an output */
    PORTC.DDR.BIT.B6 = 1 ;
    /* Set level to inactive */
    PORTC.DR.BIT.B6 = 1 ;

    /* SCK as an output */
    PORTC.DDR.BIT.B5 = 1 ;
    /* Set level to inactive */
    PORTC.DR.BIT.B5 = 1 ;

    /* Initialize SPI (per flowchart in hardware manual) */
    /* No loopback, CMos Output */
    RSPI0.SPPCR.BYTE = 0x00 ;
    /* Full Speed is 0x00  255 works */
    RSPI0.SPBR.BYTE = 0x00 ;
    /* 16-bit data 1 frame 1 chip select */
    RSPI0.SPDCR.BYTE = 0x00 ;
    /* 2 clock delay before next access to SPI device */
    RSPI0.SPCKD.BYTE = 0x00 ;
    /* 2 clock delay after de-asserting SSL */
    RSPI0.SSLND.BYTE = 0x00 ;
    /* 2 clock delay before next access to SPI device */
    RSPI0.SPND.BYTE = 0x00 ;
    /* No parity no idle interrupts */
    RSPI0.SPCR2.BYTE = 0x00 ;
    /* MSB first 8-bit data, keep SSL low */
    RSPI0.SPCMD0.WORD = 0x0700 ;
    /* Enable RSPI 3wire in master mode with RSPI Enable Transmit Only and Interupt */
    RSPI0.SPCR.BYTE = 0x6B ;
    /* SSL3A Polarity */
    RSPI0.SSLP.BYTE = 0x08 ;
    /* One frame. */
    RSPI0.SPSCR.BYTE = 0x00 ;

    //init display buffer
    display_clear_buffer();

    OS_ERR  err;
    OSTaskCreate((OS_TCB     *)&DisplayTaskTCB,
                 (CPU_CHAR   *)"Display task",
                 (OS_TASK_PTR ) task,
                 (void       *) 0,
                 (OS_PRIO     ) DISPLAY_TASK_PRIO,
                 (CPU_STK    *)&DisplayTaskStk[0],
                 (CPU_STK_SIZE) DISPLAY_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) DISPLAY_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

static void task_init(void) {
    OS_ERR  err;
    uint8_t page = 0;
    uint8_t c = 0;

    // Set bias booster
    send_command(LCD_FUNCTION_ONE); // 0x29
    send_command(0x16); // bias
    send_command(LCD_FUNCTION_THREE); // 0x2B
    send_command(0x9B); // boost

    // Set VO range
    send_command(LCD_FUNCTION_ZERO);
    send_command(0x04); // ?
    send_command(LCD_FUNCTION_ONE);
    send_command(0xdF); // ?

    OSTimeDly(60, OS_OPT_TIME_TIMEOUT, &err); // todo 60 ms

    // SET FRAME RATE
    // set frame rate
    send_command(LCD_FUNCTION_THREE);
    send_command(0x0F); // frame rate 137
    // set page
    page = 0;
    send_command(LCD_FUNCTION_ZERO);
    //send_command((page | 0x40) & 0x4F);
    send_command(0x40); // hardcode for now
    // set char
    c = 0;
    send_command(LCD_FUNCTION_ZERO);
    //send_command((c | 0x80) & 0xFF);
    send_command(0x80);

    // SET NORMAL SCREEN
    send_command(LCD_FUNCTION_ZERO);
    send_command(0x0c); // 0x0c - normal, 0x0d - inverse, 0x09 - all ON

    // CLEAR
    // set line
    send_command(LCD_FUNCTION_ZERO);
    send_command(0x40);
    send_command(0x04); //
    //
    for (int counter = 0; counter < 8; counter++) {
        // set page
        page = counter;
        send_command(LCD_FUNCTION_ZERO);
        send_command((page | 0x40) & 0x4F);
        // set char
        send_command(LCD_FUNCTION_ZERO);
        send_command((c | 0x80) & 0xFF);
        int column = 0;
        for (column=0; column<128; column++)    {
            send_data(0x00);
        }
    }
    page = 0;
    send_command(LCD_FUNCTION_ZERO);
    send_command((page | 0x40) & 0x4F);
    // set char
    send_command(LCD_FUNCTION_ZERO);
    send_command((0 | 0x80) & 0xFF);
}

static void task_send_data(void) {
    size_t dbi = 0; // display buffer index

    for (int counter = 0; counter < 8; counter++) {
        // set line
        send_command(LCD_FUNCTION_ZERO);
        send_command(0x40);
        send_command(0x04); //
        // set char
        send_command(LCD_FUNCTION_ZERO);
        send_command((0 | 0x80) & 0xFF);
        // set page
        send_command(LCD_FUNCTION_ZERO);
        send_command((counter | 0x40) & 0x4F);
        for (int column=0; column<16; column++) {
            if (column > 11) {
                for (int k = 0; k < 8; k++)
                    send_data(0xFF);
            } else {
                for (int k = 0; k < 8; k++)
                    send_data(display_buffer[dbi++]);
            }
        }
    }
}

static void task(void *p_arg) {
    OS_ERR  err;
    OS_TICK start_time;
    OS_TICK time_after_delay;

    task_init();
    start_time = OSTimeGet(&err);
    while (1) {
        task_send_data();
        render();
        OSTimeDly(FRAME_PERIOD, OS_OPT_TIME_PERIODIC, &err);
        time_after_delay = OSTimeGet(&err);
        if ((time_after_delay - start_time) > FRAME_PERIOD) {
            ; // frame period is missed. Check if needed.
        }
        start_time = time_after_delay;
    }
}
