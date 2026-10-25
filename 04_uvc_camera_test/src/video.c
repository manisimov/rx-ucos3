#include "video.h"

// Luma values of BT.601 video range: host converts 16 to black and 235 to white
#define LUMA_BLACK   (16u)
#define LUMA_WHITE   (235u)
#define CHROMA_NONE  (128u)   // U = V = 128: no colour

#define CHESS_SQUARE (20u)    // 8 x 6 squares
#define STRIPE_WIDTH (16u)    // 10 vertical stripes

#ifdef VIDEO_FMT_GREY
#define BYTES_PER_PIXEL (1u)
const USBD_UVC_FMT video_fmt = {
    {USBD_UVC_GUID_Y800}, 8u, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FRAME_INTERVAL
};
#else
#define BYTES_PER_PIXEL (2u)
const USBD_UVC_FMT video_fmt = {
    {USBD_UVC_GUID_YUY2}, 16u, VIDEO_WIDTH, VIDEO_HEIGHT, VIDEO_FRAME_INTERVAL
};
#endif

#define LINE_BYTES   (VIDEO_WIDTH * BYTES_PER_PIXEL)

static CPU_INT08U luma(const struct video_frame_t *f, CPU_INT32U x, CPU_INT32U y) {
    CPU_INT32U white;

    switch (f->pattern) {
    case VIDEO_PATTERN_RECT:
        white = ((int)x >= f->rect_x) && ((int)x < f->rect_x + VIDEO_RECT_WIDTH) &&
                ((int)y >= f->rect_y) && ((int)y < f->rect_y + VIDEO_RECT_HEIGHT);
        break;
    case VIDEO_PATTERN_CHESS:
        white = ((x / CHESS_SQUARE) + (y / CHESS_SQUARE)) & 1u;
        break;
    default:
        white = (x / STRIPE_WIDTH) & 1u;
        break;
    }

    return white ? LUMA_WHITE : LUMA_BLACK;
}

void video_fill(void *p_arg, CPU_INT32U offset, CPU_INT08U *p_buf, CPU_INT32U len) {
    const struct video_frame_t *f = p_arg;
    CPU_INT32U x = offset % LINE_BYTES; // byte in line
    CPU_INT32U y = offset / LINE_BYTES;

    while (len--) {
#ifdef VIDEO_FMT_GREY
        *p_buf++ = luma(f, x, y);
#else
        // YUY2: Y0 U Y1 V, chroma bytes are at odd positions
        *p_buf++ = (x & 1u) ? CHROMA_NONE : luma(f, x >> 1, y);
#endif
        if (++x == LINE_BYTES) {
            x = 0;
            y++;
        }
    }
}
