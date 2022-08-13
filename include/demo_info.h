#ifndef DEMO_INFO_H
#define DEMO_INFO_H

#include "demo.h"

typedef struct {
    enum Game game;
    uint32_t demo_protocol;
    uint32_t network_protocol;
    uint8_t MSSC;
    bool has_tick_interval;
    float tick_interval;

    const DemoMessageSettings* msg_settings;
    const NetSvcMessageSettings* net_msg_settings;

    uint8_t parse_level;
    bool debug_mode;
} DemoInfo;

extern DemoInfo demo_info;


#endif