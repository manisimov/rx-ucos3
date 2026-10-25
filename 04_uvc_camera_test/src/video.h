#ifndef VIDEO_H
#define VIDEO_H

#include <usbd_uvc.h>

// Video mode. Only one mode is reported to host.
// Define VIDEO_FMT_GREY to send 8 bit grey (Y800) instead of YUY2. It halves
// USB traffic, but not every PC application supports it (YUY2 works everywhere).
#define VIDEO_WIDTH            (160u)
#define VIDEO_HEIGHT           (120u)
#define VIDEO_FPS              (10u)
#define VIDEO_FRAME_INTERVAL   (10000000u / VIDEO_FPS)   // 100 ns units

#define VIDEO_RECT_WIDTH       (24)
#define VIDEO_RECT_HEIGHT      (18)

enum {
    VIDEO_PATTERN_RECT = 0,     // white rectangle on black background
    VIDEO_PATTERN_CHESS,
    VIDEO_PATTERN_STRIPES,
    VIDEO_PATTERN_SIZE
};

// Frame content, snapshot is taken once per frame
struct video_frame_t {
    int pattern;
    int rect_x;                 // top left corner of rectangle, pixels
    int rect_y;
};

extern const USBD_UVC_FMT video_fmt;

// Generates frame bytes, used as USBD_UVC_FILL_FNCT.
// p_arg points to struct video_frame_t.
void video_fill(void *p_arg, CPU_INT32U offset, CPU_INT08U *p_buf, CPU_INT32U len);

#endif // VIDEO_H
