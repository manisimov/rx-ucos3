#ifndef USB_DEVICE_H
#define USB_DEVICE_H

#include  <usbd_core.h>

extern const USBD_DEV_CFG  USBD_DevCfg;
extern const USBD_DRV_CFG  USBD_DrvCfg;
extern const USBD_DRV_BSP_API  USBD_DrvBSP;

void  BSP_USBD_Controller_IntHandler(void      *p_arg);

#endif // USB_DEVICE_H
