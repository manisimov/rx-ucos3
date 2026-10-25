This example is AI generated on the base of examples 02 and 03.

Board is visible on PC as USB camera (UVC 1.0, bulk streaming):
160x120, 10 fps, uncompressed YUY2 with no colour (U = V = 128).
Images are generated on the fly, no frame buffer is used.

Default image is white rectangle on black background. Rectangle is moved by
accelerometer (ADXL345, code from example 03) like a ball on tilted table:
tilt sets speed and direction. Board must lie flat at power up, this position
is taken as level. SWITCH 2 takes current position as level and centres rectangle.
SWITCH 1 switches image: rectangle / chessboard / vertical stripes.
If rectangle moves in wrong direction change IMAGE_X / IMAGE_Y in main.c.

UVC class (minimal subset) is in ucos/uC-USBD/Class/Video.
Define VIDEO_FMT_GREY (src/video.h) to send 8 bit grey (Y800) instead of YUY2.

LEDs: LED8 - alive, LED4 - blinks while streaming, LED10 - toggles on image switch,
LED12 - UVC or accelerometer init error.

Check on PC:

        ffplay -f v4l2 -input_format yuyv422 -video_size 160x120 /dev/video2

Debug output:

        sudo picocom -b 115200 /dev/ttyUSB0
