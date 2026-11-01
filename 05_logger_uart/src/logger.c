#include "logger.h"
#include "logger_reader.h"
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <os.h>

#define BUFFER_SIZE           (256u)
#define WRITE_BUFFER_SIZE     (128u)
#define TEXT_BUFFER_SIZE      (64u)
#define TIMESTAMP_BUFFER_SIZE (32u)

static void logger_write(const char*format, va_list va);

struct logger {
    OS_MUTEX mutex;
    uint8_t buffer[BUFFER_SIZE];
    uint32_t ri;
    uint32_t wi;
    OS_Q message_queue;
    uint32_t is_empty : 1;

    // Read realated
    uint16_t cur_read_index;
    uint16_t cur_read_remains;

    // Write related
    uint8_t write_buffer[WRITE_BUFFER_SIZE];
    uint8_t text_buffer[TEXT_BUFFER_SIZE];
    uint8_t timestamp_buffer[TIMESTAMP_BUFFER_SIZE];
};

struct logger logger;

void logger_init(void) {
    OS_ERR err;

    OSMutexCreate(&logger.mutex, "Logger mutex", &err);
    if (err != OS_ERR_NONE)
        return;
    memset(logger.buffer, 0, BUFFER_SIZE);
    logger.ri = 0;
    logger.wi = 0;
    logger.is_empty = 1;
    OSQCreate(&logger.message_queue, "Logger queue", 20, &err);
    if (err != OS_ERR_NONE)
        return;
    logger_reader_init();
}

// message [H - size, L - start index]

/* API for writer side */
void log_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    logger_write(format, args);
    va_end(args);
}

static void logger_write(const char *format, va_list va) {
    OS_ERR err;
    int bytes;

    OSMutexPend(&logger.mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);

    // text, location, timestamp => snprintf to write_buffer
    OS_TICK timestamp = OSTimeGet(&err);
    snprintf(logger.timestamp_buffer, sizeof(logger.timestamp_buffer),
             "[%d]", timestamp);

    vsnprintf(logger.text_buffer, sizeof(logger.text_buffer), format, va);

    bytes = snprintf(logger.write_buffer, sizeof(logger.write_buffer),
                     "%s %s\n",
                     logger.timestamp_buffer,
                     logger.text_buffer);

    // check for available space in buffer
    uint32_t r_avail;
    uint32_t l_avail;
    uint32_t avail_size;
    if (logger.ri == logger.wi) {
        if (logger.is_empty) {
            r_avail = BUFFER_SIZE;
            l_avail = 0;
        } else {
            r_avail = 0;
            l_avail = 0;
        }
    } else if (logger.wi > logger.ri) {
        r_avail = BUFFER_SIZE - logger.wi;
        l_avail = logger.ri;
    } else { // wi < ri
        r_avail = logger.ri - logger.wi;
        l_avail = 0;
    }
    avail_size = r_avail + l_avail;

    // write buffer to ring buffer, and update wi
    uint32_t written = 0;
    uint32_t start_index = logger.wi;
    for (uint32_t i = 0; i < avail_size, i < bytes; i++) {
        logger.buffer[logger.wi] = logger.write_buffer[i];
        written++;
        logger.wi++;
        if (logger.wi >= BUFFER_SIZE)
            logger.wi = 0;
    }

    // Send message to queue
    // message [H - size, L - start index]
    void *msg_p = (void*)((start_index & 0xFFFFu) | (written << 16));
    if (written != 0) {
        logger.is_empty = 0;
        OSQPost(&logger.message_queue,
                msg_p,
                0,
                OS_OPT_POST_FIFO,
                &err);
    }

    OSMutexPost(&logger.mutex,
                OS_OPT_POST_NONE,
                &err);
}

// log set level
// log_debug
// log_warn
// ...

/* API for reader side */
void logger_read_start(void) {
    void *msg;
    OS_MSG_SIZE msg_size;
    CPU_TS timestamp;
    OS_ERR err;

    msg = OSQPend(&logger.message_queue,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  &msg_size,
                  &timestamp,
                  &err);

    logger.cur_read_index = ((uint32_t)msg) & 0xFFFF;
    logger.cur_read_remains = ((uint32_t)msg >> 16) & 0xFFFF;
}

int logger_read_next(void) {
    int res = -1;

    if (logger.cur_read_remains == 0) {
        return res;
    }

    res = logger.buffer[logger.cur_read_index];
    logger.cur_read_index++;
    if (logger.cur_read_index >= BUFFER_SIZE) {
        logger.cur_read_index = 0;
    }
    logger.cur_read_remains--;

    return res;
}

void logger_read_end(void) {
    OS_ERR err;

    OSMutexPend(&logger.mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                NULL,
                &err);

    logger.ri = logger.cur_read_index;
    if (logger.ri == logger.wi) {
        logger.is_empty = 1;
    }

    OSMutexPost(&logger.mutex,
                OS_OPT_POST_NONE,
                &err);
}
