#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#include <stdint.h>

typedef struct {
    uint32_t demo;
    uint32_t sar_dll;
} SourceAutoRecordChecksum;

typedef struct {
    uint8_t id;
    union {
        SourceAutoRecordChecksum checksum;
        uint64_t signature;
    } data;
} SourceAutoRecordMessage;

typedef struct {
    uint32_t cursor_x;
    uint32_t cursor_y;
    bool has_sar_message;
    SourceAutoRecordMessage sar_message;
} RadialMouseMenuCallback;

#endif
