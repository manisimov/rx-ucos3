#include "hw.h"
#include <os.h>
#include "leds.h"

#define  FIRST_TASK_STK_SIZE                      256u
#define  FIRST_TASK_PRIO                            4u

static  OS_TCB       FirstTaskTCB;
static  CPU_STK      FirstTaskStk[FIRST_TASK_STK_SIZE];

static  void  FirstTask (void *p_arg);

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

extern void    hw_os_tick_init(void);

static  void  FirstTask (void *p_arg) {
    OS_ERR  err;

    leds_init();
    leds_off(LEDS_LED10);
    leds_off(LEDS_LED12);
    hw_init();
    CPU_Init();
    hw_os_tick_init();

    while (1) {
        counter++;
        leds_toggle(LEDS_LED8);
        OSTimeDlyHMSM(0u, 0u, 2u, 0u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}
