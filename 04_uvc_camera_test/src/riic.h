#ifndef RIIC_H
#define RIIC_H

#include <stdint.h>
#include <stddef.h>

// Polling driver for RIIC0 (P12 - SCL0, P13 - SDA0) in master mode.
// Based on BSP_RIIC0_xxx functions from Renesas/Micrium YRDKRX62N BSP.
// Not thread safe: use it from one task only.

void riic_init(void);
// Return 0 on success, -1 on NACK or timeout
int riic_master_write(uint8_t addr, const uint8_t *data, size_t len);
int riic_master_read(uint8_t addr, uint8_t *data, size_t len);

#endif // RIIC_H
