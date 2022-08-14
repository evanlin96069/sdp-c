#ifndef CUSTOM_MESSAGE_H
#define CUSTOM_MESSAGE_H

#include <stdint.h>

typedef struct {
    uint8_t id;
    uint32_t demo_checksum;
    uint32_t sar_checksum;
} SourceAutoRecordMessage;

typedef struct {
    uint32_t cursor_x;
    uint32_t cursor_y;
    bool has_sar_message;
    SourceAutoRecordMessage sar_message;
} RadialMouseMenuCallback;

#endif