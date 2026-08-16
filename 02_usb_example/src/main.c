#include "hw.h"
#include <os.h>
#include  <usbd_hid.h>
#include "leds.h"
#include <app_usbd.h>
#include "usb_device.h"
#include <usbd_drv_rx600.h>
#include <stddef.h>

#define  FIRST_TASK_STK_SIZE                      512u
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

static  void  App_USBD_EventReset  (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventSuspend(CPU_INT08U  dev_nbr);
static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val);
static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val);
static  void  App_USBD_EventConn   (CPU_INT08U  dev_nbr);
static  void  App_USBD_EventDisconn(CPU_INT08U  dev_nbr);


static  USBD_BUS_FNCTS  App_USBD_BusFncts = {
    App_USBD_EventReset,
    App_USBD_EventSuspend,
    App_USBD_EventResume,
    App_USBD_EventCfgSet,
    App_USBD_EventCfgClr,
    App_USBD_EventConn,
    App_USBD_EventDisconn
};

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportGet(CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);

static  CPU_BOOLEAN  App_USBD_HID_FeatureReportSet(CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);

static  CPU_INT08U   App_USBD_HID_ProtocolGet     (CPU_INT08U   class_nbr,
                                                   USBD_ERR    *p_err);

static  void         App_USBD_HID_ProtocolSet     (CPU_INT08U   class_nbr,
                                                   CPU_INT08U   protocol,
                                                   USBD_ERR    *p_err);

static  void         App_USBD_HID_ReportSet       (CPU_INT08U   class_nbr,
                                                   CPU_INT08U   report_id,
                                                   CPU_INT08U  *p_report_buf,
                                                   CPU_INT16U   report_len);

static  void         App_USBD_HID_MouseSetReport  (CPU_INT08U  *p_buf);


static  USBD_HID_CALLBACK  App_USBD_HID_Callback = {
    App_USBD_HID_FeatureReportGet,
    App_USBD_HID_FeatureReportSet,
    App_USBD_HID_ProtocolGet,
    App_USBD_HID_ProtocolSet,
    App_USBD_HID_ReportSet
};

#define APP_USBD_HID_REPORT_LEN (4u)
static  CPU_INT08U  App_USBD_HID_Protocol;
static  CPU_INT08U  App_USBD_HID_ReportBuf[APP_USBD_HID_REPORT_LEN];
static  CPU_INT32S  App_USBD_HID_X;
static  CPU_INT32S  App_USBD_HID_Y;

static  OS_TCB      App_USBD_HID_MouseTaskTCB;
static  CPU_STK     App_USBD_HID_MouseTaskStk[APP_CFG_USBD_HID_TASK_STK_SIZE];

static  void         App_USBD_HID_MouseTask       (void        *p_arg);


static  void  FirstTask (void *p_arg) {
    OS_ERR  err;
    USBD_ERR usb_err;

    leds_init();
    leds_off(LEDS_LED10);
    leds_off(LEDS_LED12);
    hw_init();
    CPU_Init();
    hw_os_tick_init();

    Mem_Init();

    USBD_Init(&usb_err);

    App_USBD_HID_X = 101;
    App_USBD_HID_Y = 101;

    CPU_INT08U dev_num;
    dev_num = USBD_DevAdd(&USBD_DevCfg,
                          &App_USBD_BusFncts,
                          &USBD_DrvAPI_RX600,
                          &USBD_DrvCfg,
                          &USBD_DrvBSP,
                          &usb_err);

    USBD_DevSelfPwrSet(dev_num, DEF_NO, &usb_err);

    CPU_INT08U cfg_num;
    cfg_num = USBD_CfgAdd(dev_num,
                          USBD_DEV_ATTRIB_SELF_POWERED,
                          100u,
                          USBD_DEV_SPD_FULL,
                          "FS configuration",
                             &usb_err);

    USBD_HID_Init(&usb_err);

    CPU_INT08U  class_nbr;

    volatile size_t test = usb_device_get_HID_ReportDesc_size();

    class_nbr = USBD_HID_Add(             USBD_HID_SUBCLASS_BOOT,
                                          USBD_HID_PROTOCOL_MOUSE,
                                          USBD_HID_COUNTRY_CODE_NOT_SUPPORTED,
					  usb_device_get_HID_ReportDesc(),
                                          usb_device_get_HID_ReportDesc_size(),
                            (CPU_INT08U *)0,
                                          0u,
                                          2u,
                                          2u,
                                          DEF_YES,
                                         &App_USBD_HID_Callback,
                                         &usb_err);

    USBD_HID_CfgAdd(class_nbr, dev_num, cfg_num, &usb_err);

    OSTaskCreate(                 &App_USBD_HID_MouseTaskTCB,
                                  "USB Device HID Mouse",
                                   App_USBD_HID_MouseTask,
                 (void *)(CPU_ADDR)class_nbr,
                                   APP_CFG_USBD_HID_MOUSE_TASK_PRIO,
                                  &App_USBD_HID_MouseTaskStk[0],
                                   APP_CFG_USBD_HID_TASK_STK_SIZE / 10u,
                                   APP_CFG_USBD_HID_TASK_STK_SIZE,
                                   0u,
                                   0u,
                 (void *)          0,
                                   OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                                  &err);

    USBD_DevStart(dev_num, &usb_err);



    while (1) {
        counter++;
        leds_toggle(LEDS_LED8);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u,
                      OS_OPT_TIME_HMSM_STRICT,
                      &err);
    }
}


static  void  App_USBD_EventReset (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}

static  void  App_USBD_EventSuspend (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}

static  void  App_USBD_EventResume (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}

static  void  App_USBD_EventCfgSet (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)dev_nbr;
    (void)cfg_val;
}

static  void  App_USBD_EventCfgClr (CPU_INT08U  dev_nbr,
                                    CPU_INT08U  cfg_val)
{
    (void)dev_nbr;
    (void)cfg_val;
}

static  void  App_USBD_EventConn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}

static  void  App_USBD_EventDisconn (CPU_INT08U  dev_nbr)
{
    (void)dev_nbr;
}



static  CPU_BOOLEAN  App_USBD_HID_FeatureReportGet (CPU_INT08U   class_nbr,
                                                    CPU_INT08U   report_id,
                                                    CPU_INT08U  *p_report_buf,
                                                    CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;

    Mem_Clr(p_report_buf, report_len);

    return (DEF_OK);
}


static  CPU_BOOLEAN  App_USBD_HID_FeatureReportSet (CPU_INT08U   class_nbr,
                                                    CPU_INT08U   report_id,
                                                    CPU_INT08U  *p_report_buf,
                                                    CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;
    (void)p_report_buf;
    (void)report_len;

    return (DEF_OK);
}


static  CPU_INT08U  App_USBD_HID_ProtocolGet (CPU_INT08U   class_nbr,
                                              USBD_ERR    *p_err)
{
    (void)class_nbr;

   *p_err = USBD_ERR_NONE;

    return (App_USBD_HID_Protocol);
}

static  void  App_USBD_HID_ProtocolSet (CPU_INT08U   class_nbr,
                                        CPU_INT08U   protocol,
                                        USBD_ERR    *p_err)
{
    (void)class_nbr;

    App_USBD_HID_Protocol = protocol;

   *p_err = USBD_ERR_NONE;
}

static  void  App_USBD_HID_ReportSet (CPU_INT08U   class_nbr,
                                      CPU_INT08U   report_id,
                                      CPU_INT08U  *p_report_buf,
                                      CPU_INT16U   report_len)
{
    (void)class_nbr;
    (void)report_id;
    (void)p_report_buf;
    (void)report_len;
}

static void  App_USBD_HID_MouseSetReport (CPU_INT08U  *p_buf)
{
    CPU_SR_ALLOC();


    p_buf[0] = 0;
    p_buf[1] = 0;

    CPU_CRITICAL_ENTER();
    p_buf[2] = (App_USBD_HID_X > 0) ? 50 : -50;
    p_buf[3] = (App_USBD_HID_Y > 0) ? 50 : -50;

    App_USBD_HID_X = !App_USBD_HID_X;
    App_USBD_HID_Y = !App_USBD_HID_Y;
    CPU_CRITICAL_EXIT();
}

static  void  App_USBD_HID_MouseTask (void  *p_arg)
{
    CPU_INT08U   class_nbr;
    CPU_BOOLEAN  conn;
    USBD_ERR     err;
    OS_ERR       os_err;

    class_nbr = (CPU_INT08U)(CPU_ADDR)p_arg;

    while (DEF_TRUE) {
        conn = USBD_HID_IsConn(class_nbr);
        while (conn != DEF_YES) {
            OSTimeDlyHMSM(0u, 0u, 0u, 250u, OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
            (void)os_err;
            conn = USBD_HID_IsConn(class_nbr);
        }

        App_USBD_HID_MouseSetReport(&App_USBD_HID_ReportBuf[0u]);

        (void)USBD_HID_Wr( class_nbr,
                          &App_USBD_HID_ReportBuf[0],
                           APP_USBD_HID_REPORT_LEN,
                           0u,
                          &err);
        OSTimeDlyHMSM(0u, 0u, 0u, 100u, OS_OPT_TIME_HMSM_NON_STRICT, &os_err);
        (void)os_err;
    }
}
