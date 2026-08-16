#include  <usbd_core.h>
#include "iodefine.h"
#include "usb_device.h"
//#include  <bsp_usbd_template.h>


/*
*********************************************************************************************************
*                                      USB DEVICE CONFIGURATION
*********************************************************************************************************
*/

const USBD_DEV_CFG  USBD_DevCfg = {
    0xFFFE,                                                     /* Vendor  ID.                                          */
    0x5011,                                                     /* Product ID.                                          */
    0x0100,                                                     /* Device release number.                               */
   "MIH MANUFACTURER",                                      /* Manufacturer  string.                                */
   "MIH PRODUCT",                                           /* Product       string.                                */
   "1234567890ABCDEF",                                          /* Serial number string.                                */
    USBD_LANG_ID_ENGLISH_US                                     /* String language ID.                                  */
};


/*
*********************************************************************************************************
*                                USB DEVICE ENDPOINT INFORMATION TABLE
*********************************************************************************************************
*/

/* static USBD_DRV_EP_INFO  USBD_DrvEP_InfoTbl_Template[] = { */
/*     {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_OUT, 0u,   64u}, */
/*     {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_IN,  0u,   64u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 1u, 1024u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  1u, 1024u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 2u, 1024u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  2u, 1024u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 3u, 1024u}, */
/*     {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  3u, 1024u}, */
/*     {DEF_BIT_NONE                                                                                 ,   0u,    0u} */
/* }; */

/* static USBD_DRV_EP_INFO  USBD_DrvEP_InfoTbl[] = { */
/*     {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_OUT, 0u,   64u}, */
/*     {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_IN,  0u,   64u}, */
/*     {DEF_BIT_NONE                                                                                 ,   0u,    0u} */
/* }; */

static USBD_DRV_EP_INFO  USBD_DrvEP_InfoTbl[] = {
    {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_OUT, 0u,   64u},
    {USBD_EP_INFO_TYPE_CTRL                                                   | USBD_EP_INFO_DIR_IN,  0u,   64u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 1u, 1024u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  1u, 1024u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 2u, 1024u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  2u, 1024u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_OUT, 3u, 1024u},
    {USBD_EP_INFO_TYPE_ISOC | USBD_EP_INFO_TYPE_BULK | USBD_EP_INFO_TYPE_INTR | USBD_EP_INFO_DIR_IN,  3u, 1024u},
    {DEF_BIT_NONE                                                                                 ,   0u,    0u}
};


/*
*********************************************************************************************************
*                                   USB DEVICE DRIVER CONFIGURATION
*********************************************************************************************************
*/

const USBD_DRV_CFG  USBD_DrvCfg = {
    0x000A0000,                                                 /* Base addr   of device controller hw registers.       */
    0x00000000,                                                 /* Base addr   of device controller dedicated mem.      */
             0u,                                                /* Size        of device controller dedicated mem.      */
    USBD_DEV_SPD_FULL,                                          /* Speed       of device controller.                    */
    USBD_DrvEP_InfoTbl                                 /* EP Info tbl of device controller.                    */   // todo ????
};


/////////////////////// BSP ///////////////////////////

static  USBD_DRV  *BSP_USBD_Controller_DrvPtr;



static  void  BSP_USBD_Template_Init        (USBD_DRV  *p_drv);
static  void  BSP_USBD_Template_Connect     (void            );
static  void  BSP_USBD_Template_Disconnect  (void            );
void  BSP_USBD_Controller_IntHandler(void      *p_arg);


/*
*********************************************************************************************************
*                                   USB DEVICE DRIVER BSP INTERFACE
*********************************************************************************************************
*/

const USBD_DRV_BSP_API  USBD_DrvBSP = {
    BSP_USBD_Template_Init,
    BSP_USBD_Template_Connect,
    BSP_USBD_Template_Disconnect
};




/*
*********************************************************************************************************
*                                      BSP_USBD_Template_Init()
*
* Description : USB device controller board-specific initialization.
*
* Argument(s) : p_drv    Pointer to device driver structure.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_USBD_Template_Init (USBD_DRV  *p_drv)
{
    (void)p_drv;

    /* $$$$ If ISR does not support argument passing, save reference to USBD_DRV in a local global variable. */
    BSP_USBD_Controller_DrvPtr = p_drv;

    /* $$$$ This function performs all operations that the device controller cannot do. Typical operations are: */
    /* $$$$ Enable device control registers and bus clock [mandatory]. */
    /* $$$$ Configure main USB device interrupt in interrupt controller (e.g. registering BSP ISR) [mandatory]. */
    /* $$$$ Disable device transceiver clock [optional]. */
    /* $$$$ Configure I/O pins [if necessary]. */

    // Setup pins for input (DDR). Enable input buffer for them (ICR)
    PORT2.DDR.BIT.B5 = 0;
    PORT2.ICR.BIT.B5 = 1; // P25 - USB0_DPRPD
    PORT2.DDR.BIT.B2 = 0;
    PORT2.ICR.BIT.B2 = 1; // P22 - USB0_DRPD
    PORT1.DDR.BIT.B6 = 0;
    PORT1.ICR.BIT.B6 = 1; // P16 - USB0_VBUS

    SYSTEM.MSTPCRB.BIT.MSTPB19 = 0; // enable clocks to usb module
    IOPORT.PFKUSB.BIT.USBE = 1;     // enable usb pins
    IOPORT.PFKUSB.BIT.USBMD = 0;    // setup function mode.
                                    //(function means usb device, as oposite to usb host)
    IR(USB0, USBI0) = 0;  // clear interrupt request
    IPR(USB0, USBI0) = 6; // interrupt priority
    IEN(USB0, USBI0) = 1; // interrupt enable
}


/*
*********************************************************************************************************
*                                     BSP_USBD_Template_Connect()
*
* Description : Connect pull-up on DP.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_USBD_Template_Connect (void)
{
    /* $$$$ Enable device transceiver clock [optional]. */
    /* $$$$ Enable pull-up resistor (this operation may be done in the driver) [mandatory]. */
    /* $$$$ Enable main USB device interrupt [mandatory]. */
}


/*
*********************************************************************************************************
*                                   BSP_USBD_Template_Disconnect()
*
* Description : Disconnect pull-up on DP.
*
* Argument(s) : none.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/

static  void  BSP_USBD_Template_Disconnect (void)
{
    /* $$$$ Disable device transceiver clock [optional]. */
    /* $$$$ Disable pull-up resistor (this operation may be done in the driver) [mandatory]. */
    /* $$$$ Disable main USB device interrupt [mandatory]. */
}


/*
*********************************************************************************************************
*                                  BSP_USBD_Controller_IntHandler()
*
* Description : USB device interrupt handler.
*
* Argument(s) : p_arg    Interrupt handler argument.
*
* Return(s)   : none.
*
* Note(s)     : none.
*********************************************************************************************************
*/



void  BSP_USBD_Controller_IntHandler (void  *p_arg)
{
    USBD_DRV      *p_drv;
    USBD_DRV_API  *p_drv_api;


                                                                /* Get a reference to USBD_DRV.                         */
    /* $$$$ If ISR does not support argument passing, get USBD_DRV from a global variable initialized in BSP_USBD_Template_Init(). */
    p_drv = BSP_USBD_Controller_DrvPtr;
    /* $$$$ Otherwise if ISR supports argument passing, get USBD_DRV from the argument. Do not forget to pass USBD_DRV structure when registering the BSP ISR in BSP_USBD_Template_Init(). */
    //p_drv = (USBD_DRV *)p_arg;

    p_drv_api = p_drv->API_Ptr;                                 /* Get a reference to USBD_DRV_API.                     */
    p_drv_api->ISR_Handler(p_drv);                              /* Call the USB Device driver ISR.                      */
}

#include <usbd_hid_report.h>


static CPU_INT08U  usb_device_HID_ReportDesc[] = {               /* HID Mouse report                                     */
    USBD_HID_GLOBAL_USAGE_PAGE           + 1, USBD_HID_USAGE_PAGE_GENERIC_DESKTOP_CONTROLS,
    USBD_HID_LOCAL_USAGE                 + 1, USBD_HID_CA_MOUSE,
    USBD_HID_MAIN_COLLECTION             + 1, USBD_HID_COLLECTION_APPLICATION,
        USBD_HID_LOCAL_USAGE             + 1, USBD_HID_CP_POINTER,
        USBD_HID_MAIN_COLLECTION         + 1, USBD_HID_COLLECTION_PHYSICAL,
            USBD_HID_GLOBAL_USAGE_PAGE   + 1, USBD_HID_USAGE_PAGE_BUTTON,
            USBD_HID_LOCAL_USAGE_MIN     + 1, 0x01,
            USBD_HID_LOCAL_USAGE_MAX     + 1, 0x03,
            USBD_HID_GLOBAL_LOG_MIN      + 1, 0x00,
            USBD_HID_GLOBAL_LOG_MAX      + 1, 0x01,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x03,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x01,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_DATA | USBD_HID_MAIN_VARIABLE | USBD_HID_MAIN_ABSOLUTE,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x01,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x0D,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_CONSTANT,
            USBD_HID_GLOBAL_USAGE_PAGE   + 1, USBD_HID_USAGE_PAGE_GENERIC_DESKTOP_CONTROLS,
            USBD_HID_LOCAL_USAGE         + 1, USBD_HID_DV_X,
            USBD_HID_LOCAL_USAGE         + 1, USBD_HID_DV_Y,
            USBD_HID_GLOBAL_LOG_MIN      + 1, 0x81,
            USBD_HID_GLOBAL_LOG_MAX      + 1, 0x7F,
            USBD_HID_GLOBAL_REPORT_SIZE  + 1, 0x08,
            USBD_HID_GLOBAL_REPORT_COUNT + 1, 0x02,
            USBD_HID_MAIN_INPUT          + 1, USBD_HID_MAIN_DATA | USBD_HID_MAIN_VARIABLE | USBD_HID_MAIN_RELATIVE,
        USBD_HID_MAIN_ENDCOLLECTION,
    USBD_HID_MAIN_ENDCOLLECTION
};

CPU_INT08U *usb_device_get_HID_ReportDesc(void) {
    return &usb_device_HID_ReportDesc[0];
}

size_t usb_device_get_HID_ReportDesc_size(void) {
    return sizeof(usb_device_HID_ReportDesc);
}
