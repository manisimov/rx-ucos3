#ifndef  APP_CFG_H
#define  APP_CFG_H

#include <lib_def.h>

#define USBD_OS_CFG_CORE_TASK_STK_SIZE (512u)
#define USBD_OS_CFG_CORE_TASK_PRIO     (2u) // TODO ???

#define USBD_HID_OS_CFG_TMR_TASK_STK_SIZE (512u)
#define USBD_HID_OS_CFG_TMR_TASK_PRIO     (9u) // TODO ???

#define APP_CFG_VIDEO_TASK_STK_SIZE (512u)
#define APP_CFG_VIDEO_TASK_PRIO     (10u)

#define APP_CFG_USBD_EN  DEF_ENABLED

#endif // APP_CFG_H
