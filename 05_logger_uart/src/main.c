#include "hw.h"
#include <os.h>
#include "leds.h"
#include "logger.h"

extern void    hw_os_tick_init(void);

#define  FIRST_TASK_STK_SIZE                      256u
#define  FIRST_TASK_PRIO                            4u

#define  SECOND_TASK_STK_SIZE                      256u
#define  SECOND_TASK_PRIO                            5u

static  OS_TCB       FirstTaskTCB;
static  CPU_STK      FirstTaskStk[FIRST_TASK_STK_SIZE];

static  OS_TCB       SecondTaskTCB;
static  CPU_STK      SecondTaskStk[SECOND_TASK_STK_SIZE];

static  void  FirstTask (void *p_arg);
static  void SecondTask(void *p_arg);

void main(void) {
    OS_ERR  err;

    CPU_IntDis();
    OSInit(&err);

    OSTaskCreate((OS_TCB     *)&FirstTaskTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"First task",
                 (OS_TASK_PTR ) FirstTask,
                 (void       *) 0,
                 (OS_PRIO     ) FIRST_TASK_PRIO,
                 (CPU_STK    *)&FirstTaskStk[0],
                 (CPU_STK_SIZE) FIRST_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) FIRST_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);

    while(1) {
        ;
    }
}

int counter = 0;



static  void  FirstTask (void *p_arg) {
    OS_ERR  err;

    leds_init();
    leds_off(LEDS_LED10);
    leds_off(LEDS_LED12);
    hw_init();
    logger_init();
    CPU_Init();
    hw_os_tick_init();

    int var = 8;
    log_warn("var value is %d", var);

    OSTaskCreate((OS_TCB     *)&SecondTaskTCB,
                 (CPU_CHAR   *)"Second task",
                 (OS_TASK_PTR ) SecondTask,
                 (void       *) 0,
                 (OS_PRIO     ) SECOND_TASK_PRIO,
                 (CPU_STK    *)&SecondTaskStk[0],
                 (CPU_STK_SIZE) SECOND_TASK_STK_SIZE / 10u,
                 (CPU_STK_SIZE) SECOND_TASK_STK_SIZE,
                 (OS_MSG_QTY  ) 0u,
                 (OS_TICK     ) 0u,
                 (void       *) 0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    while (1) {
        counter++;
        leds_toggle(LEDS_LED8);
        OSTimeDlyHMSM(0u, 0u, 0u, 200u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
        log_warn("counter value is %d", counter);
        log_warn("gigigigi");
        log_warn("some info");
    }
}

static void SecondTask(void *p_arg) {
    OS_ERR err;
    int c = 0;

    while (1) {
        OSTimeDlyHMSM(0u, 0u, 0u, 20u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
        log_warn("From second task +++++++===== %d %d", c++, err);
    }
}
