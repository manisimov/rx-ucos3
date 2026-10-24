#include <iodefine.h>
#include "hw.h"
#include "hw_os.h"
#include <os.h>
#include <stdint.h>
#include "usb_device.h"

void hw_os_tick_init(void) {
    uint16_t  cmcor;

    cmcor = hw_get_pclock_freq() / (32u * OSCfg_TickRate_Hz);

    MSTP(CMT0) = 0;                                             /* Enable CMT0 module.                                  */

    CMT.CMSTR0.BIT.STR0 = 0;                                    /* Stop timer channel 0.                                */
    CMT0.CMCR.BIT.CKS   = 1;                                    /* Set peripheral clock divider.                        */

    CMT0.CMCOR = cmcor - 1u;                                    /* Set compare-match value.                             */
    CMT0.CMCNT = 0;                                             /* Clear counter register.                              */

    IR(CMT0, CMI0)  = 0;                                        /* Clear any pending ISR.                               */
    IPR(CMT0,)      = 3;                                        /* Set interrupt priority.                              */
    IEN(CMT0, CMI0) = 1;                                        /* Enable interrupt source.                             */

    CMT0.CMCR.BIT.CMIE = 1;                                     /* Enable interrupt.                                    */

    CMT.CMSTR0.BIT.STR0 = 1;                                    /* Start timer.                                         */

}

__attribute__ ((interrupt)) void hw_os_tick_isr(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    OSTimeTick();

    OSIntExit();
}

__attribute__ ((interrupt)) void hw_os_usb_isr(void) {
    CPU_SR_ALLOC();
    CPU_CRITICAL_ENTER();
    OSIntEnter();
    CPU_CRITICAL_EXIT();

    BSP_USBD_Controller_IntHandler((void *)0);

    OSIntExit();    
}
