#ifndef ACCEL_H
#define ACCEL_H

#include <stdint.h>

// ADXL345 accelerometer on RIIC0.
// Full resolution mode: 256 LSB/g (~3.9 mg/LSB)

struct accel_t {
    int16_t x;
    int16_t y;
    int16_t z;
};

// Return 0 on success, -1 on error
int accel_init(void);
int accel_read(struct accel_t *a);
// Average several samples, board must lie still. Result is used as zero point.
int accel_calibrate(struct accel_t *zero);

#endif // ACCEL_H
