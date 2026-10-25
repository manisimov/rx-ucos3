#include "hw.h"
#include <os.h>
#include <usbd_uvc.h>
#include "leds.h"
#include <app_cfg.h>
#include "usb_device.h"
#include <usbd_drv_rx600.h>
#include "buttons.h"
#include "uart.h"
#include "video.h"
#include "accel.h"

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

static  volatile int          App_Pattern = VIDEO_PATTERN_RECT;   /* Switched by SW1                                  */
static  volatile CPU_BOOLEAN  App_Calibrate;                    /* Set by SW2, done by video task (owns I2C)            */

static  OS_TCB      App_VideoTaskTCB;
static  CPU_STK     App_VideoTaskStk[APP_CFG_VIDEO_TASK_STK_SIZE];

static  void        App_VideoTask (void  *p_arg);


static  void  FirstTask (void *p_arg) {
    OS_ERR  err;
    USBD_ERR usb_err;

    leds_init();
    buttons_init();
    leds_off(LEDS_LED10);
    leds_off(LEDS_LED12);
    hw_init();
    uart_init();                                                /* After hw_init(): baud rate depends on PCLK           */
    uart_write("\n04_uvc_camera_test\n");
    CPU_Init();
    hw_os_tick_init();

    Mem_Init();

    USBD_Init(&usb_err);

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

    USBD_UVC_Init(&video_fmt, &usb_err);
    USBD_UVC_CfgAdd(dev_num, cfg_num, &usb_err);
    if (usb_err != USBD_ERR_NONE) {
        uart_printf("UVC add failed: %d\n", usb_err);
        leds_on(LEDS_LED12);
    }

    OSTaskCreate(                 &App_VideoTaskTCB,
                                  "Video",
                                   App_VideoTask,
                 (void *)          0,
                                   APP_CFG_VIDEO_TASK_PRIO,
                                  &App_VideoTaskStk[0],
                                   APP_CFG_VIDEO_TASK_STK_SIZE / 10u,
                                   APP_CFG_VIDEO_TASK_STK_SIZE,
                                   0u,
                                   0u,
                 (void *)          0,
                                   OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR,
                                  &err);

    USBD_DevStart(dev_num, &usb_err);



    while (1) {
        counter++;
	if ((counter % 10) == 0)
            leds_toggle(LEDS_LED8);
	buttons_task();
        if (buttons_is_pressed(BUTTON_LEFT)) {                  /* SW1: next image                                      */
            App_Pattern = (App_Pattern + 1) % VIDEO_PATTERN_SIZE;
            leds_toggle(LEDS_LED10);
            uart_printf("pattern %d\n", App_Pattern);
        }
        if (buttons_is_pressed(BUTTON_MIDDLE)) {                /* SW2: current position is level                       */
            App_Calibrate = DEF_TRUE;
        }
        OSTimeDlyHMSM(0u, 0u, 0u, 10u,
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


/*
 * Accelerometer moves rectangle like a ball on tilted table: tilt sets speed.
 * Board lies flat at power up: this position is taken as level (SW2 repeats it).
 * ADXL345 in full resolution mode gives 256 LSB/g, so 1 degree of tilt is ~4.5 LSB.
 * Position is kept in 1/16 pixel units, so slow tilt still moves rectangle.
 */
#define ACCEL_DEAD_ZONE   (18)  // ~4 degrees
#define ACCEL_SPEED_MUL   (12)  // 1/16 px per second per LSB: 45 degrees ~ 120 px/s
#define POS_SHIFT         (4)

// Mapping of accelerometer axes to image axes (same as cursor in example 03).
// Change signs/axes here if rectangle moves in wrong direction.
#define IMAGE_X(a)        (-(a).x)
#define IMAGE_Y(a)        ((a).y)

#define POS_X_MAX         ((VIDEO_WIDTH  - VIDEO_RECT_WIDTH)  << POS_SHIFT)
#define POS_Y_MAX         ((VIDEO_HEIGHT - VIDEO_RECT_HEIGHT) << POS_SHIFT)

static  struct accel_t  App_Accel_Zero;
static  CPU_BOOLEAN     App_Accel_Ok;
static  CPU_INT32S      App_PosX = POS_X_MAX / 2;
static  CPU_INT32S      App_PosY = POS_Y_MAX / 2;

volatile CPU_INT32U  dbg_accel_errors;

static  CPU_INT32S  App_Accel_ToStep (CPU_INT32S  val)
{
    if (val > ACCEL_DEAD_ZONE) {
        val -= ACCEL_DEAD_ZONE;
    } else if (val < -ACCEL_DEAD_ZONE) {
        val += ACCEL_DEAD_ZONE;
    } else {
        return 0;
    }

    return val * ACCEL_SPEED_MUL / (CPU_INT32S)VIDEO_FPS;
}

static  CPU_INT32S  App_Clamp (CPU_INT32S  val, CPU_INT32S  max)
{
    if (val < 0) {
        return 0;
    }
    return (val > max) ? max : val;
}

static  void  App_Accel_Init (void)
{
    App_Accel_Ok = (accel_init() == 0) &&
                   (accel_calibrate(&App_Accel_Zero) == 0);
    if (App_Accel_Ok) {
        uart_printf("accel ok, zero: x=%d y=%d z=%d\n",
                    App_Accel_Zero.x, App_Accel_Zero.y, App_Accel_Zero.z);
    } else {
        uart_write("accel init failed\n");
        leds_on(LEDS_LED12);
    }
}

/* Called once per frame. */
static  void  App_RectUpdate (void)
{
    struct accel_t  a;
    struct accel_t  delta;


    if (!App_Accel_Ok) {
        return;
    }

    if (App_Calibrate) {
        App_Calibrate = DEF_FALSE;
        if (accel_calibrate(&App_Accel_Zero) == 0) {
            App_PosX = POS_X_MAX / 2;
            App_PosY = POS_Y_MAX / 2;
            uart_write("calibrated\n");
        }
    }

    if (accel_read(&a) != 0) {
        dbg_accel_errors++;
        return;
    }

    delta.x = a.x - App_Accel_Zero.x;
    delta.y = a.y - App_Accel_Zero.y;

    App_PosX = App_Clamp(App_PosX + App_Accel_ToStep(IMAGE_X(delta)), POS_X_MAX);
    App_PosY = App_Clamp(App_PosY + App_Accel_ToStep(IMAGE_Y(delta)), POS_Y_MAX);
}


volatile CPU_INT32U  dbg_frames;                                /* Debug counters, read them with debugger              */
volatile CPU_INT32U  dbg_frame_errors;
volatile USBD_ERR    dbg_usb_last_err;

/*
 * Sends frames while host is streaming. Frame period is kept by delay
 * after every frame (bulk endpoint: device decides when to send).
 * Image is sampled once per frame, so a frame never mixes two images.
 */
static  void  App_VideoTask (void  *p_arg)
{
    USBD_ERR     err;
    OS_ERR       os_err;
    OS_TICK      start;
    OS_TICK      elapsed;
    OS_TICK      period;
    CPU_BOOLEAN  streaming = DEF_NO;
    struct video_frame_t  frame;


    (void)p_arg;

    App_Accel_Init();

    period = (OSCfg_TickRate_Hz + VIDEO_FPS / 2u) / VIDEO_FPS;

    while (DEF_TRUE) {
        if (USBD_UVC_IsStreaming() != streaming) {
            streaming = !streaming;
            uart_write(streaming ? "stream on\n" : "stream off\n");
            leds_off(LEDS_LED4);
        }

        if (!streaming) {
            OSTimeDly(20u, OS_OPT_TIME_DLY, &os_err);
            continue;
        }

        start   = OSTimeGet(&os_err);
        App_RectUpdate();
        frame.pattern = App_Pattern;
        frame.rect_x  = App_PosX >> POS_SHIFT;
        frame.rect_y  = App_PosY >> POS_SHIFT;
        USBD_UVC_FrameTx(video_fill, &frame, &err);
        if (err == USBD_ERR_NONE) {
            dbg_frames++;
            if ((dbg_frames % VIDEO_FPS) == 0u) {
                leds_toggle(LEDS_LED4);                         /* Blinks at 0.5 Hz while streaming                     */
            }
        } else if (err != USBD_ERR_INVALID_CLASS_STATE) {      /* Not an error: stream was stopped/restarted           */
            dbg_frame_errors++;
            dbg_usb_last_err = err;
            uart_printf("frame tx err %d\n", err);
        }

        elapsed = OSTimeGet(&os_err) - start;
        if (elapsed < period) {
            OSTimeDly(period - elapsed, OS_OPT_TIME_DLY, &os_err);
        }
    }
}
