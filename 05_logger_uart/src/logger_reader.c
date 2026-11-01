#include "logger_reader.h"
#include "logger.h"
#include "uart.h"
#include <os.h>

// move to config file os_cfg_app.h?
#define  LOGGER_READER_TASK_STK_SIZE          (256u)
#define LOGGER_READER_TASK_PRIO (OS_CFG_PRIO_MAX-4u)

static OS_TCB LoggerReaderTaskTCB;
static CPU_STK LoggerReaderTaskStk[LOGGER_READER_TASK_STK_SIZE];

static void task(void *p_arg);

void logger_reader_init(void) {
    OS_ERR err;

    uart_init();

    OSTaskCreate((OS_TCB     *)&LoggerReaderTaskTCB,
                 (CPU_CHAR   *)"Logger reader task",
                 (OS_TASK_PTR ) task,
                 (void       *) 0,
                 (OS_PRIO     ) LOGGER_READER_TASK_PRIO,
                 (CPU_STK    *)&LoggerReaderTaskStk[0],
                 (CPU_STK_SIZE) LOGGER_READER_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) LOGGER_READER_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

static void task(void *p_arg) {
    while(1) {
        logger_read_start();
        int c;
        while ((c = logger_read_next()) > 0) {
            uart_putc(c);
        }
        logger_read_end();
    }
}
