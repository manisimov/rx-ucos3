#include "accel.h"
#include "riic.h"

#define ACCEL_ADDR (0x3A >> 1)

#define REG_DEVID       0x00U
#define REG_POWER_CTRL  0x2DU
#define REG_DATA_FORMAT 0x31U
#define REG_INT_SOURCE  0x30U
#define REG_FIFO_CTRL   0x38U
#define REG_DATAX0      0x32U

#define DEVID           0xE5U
#define DATA_READY      0x80U

#define CALIBRATION_SAMPLES (16)
#define DATA_READY_TIMEOUT  (1000)

static int write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};

    return riic_master_write(ACCEL_ADDR, buf, sizeof(buf));
}

static int read_regs(uint8_t reg, uint8_t *data, size_t len) {
    if (riic_master_write(ACCEL_ADDR, &reg, 1) != 0) {
        return -1;
    }
    return riic_master_read(ACCEL_ADDR, data, len);
}

// Wait for new sample. Default output data rate is 100 Hz, one I2C read
// takes ~0.3 ms, so timeout is more than 30 ms.
static int wait_data_ready(void) {
    uint8_t src;

    for (int i = 0; i < DATA_READY_TIMEOUT; i++) {
        if (read_regs(REG_INT_SOURCE, &src, 1) != 0) {
            return -1;
        }
        if (src & DATA_READY) {
            return 0;
        }
    }
    return -1;
}

int accel_init(void) {
    uint8_t id = 0;

    riic_init();

    if (read_regs(REG_DEVID, &id, 1) != 0 || id != DEVID) {
        return -1;
    }

    // Full resolution, range +/- 16g, right justified with sign extension
    if (write_reg(REG_DATA_FORMAT, 0x0B) != 0) {
        return -1;
    }
    // FIFO bypass mode
    if (write_reg(REG_FIFO_CTRL, 0x00) != 0) {
        return -1;
    }
    // Measurement mode
    if (write_reg(REG_POWER_CTRL, 0x08) != 0) {
        return -1;
    }

    return 0;
}

int accel_read(struct accel_t *a) {
    uint8_t buf[6];

    // Read all axes in one transaction, so values belong to the same sample
    if (read_regs(REG_DATAX0, buf, sizeof(buf)) != 0) {
        return -1;
    }

    a->x = (int16_t)((buf[1] << 8) | buf[0]);
    a->y = (int16_t)((buf[3] << 8) | buf[2]);
    a->z = (int16_t)((buf[5] << 8) | buf[4]);

    return 0;
}

int accel_calibrate(struct accel_t *zero) {
    struct accel_t a;
    int32_t x = 0;
    int32_t y = 0;
    int32_t z = 0;

    for (int i = 0; i < CALIBRATION_SAMPLES; i++) {
        // Data ready flag is cleared by reading data registers,
        // so every iteration gets fresh sample
        if (wait_data_ready() != 0 || accel_read(&a) != 0) {
            return -1;
        }
        x += a.x;
        y += a.y;
        z += a.z;
    }

    zero->x = x / CALIBRATION_SAMPLES;
    zero->y = y / CALIBRATION_SAMPLES;
    zero->z = z / CALIBRATION_SAMPLES;

    return 0;
}
