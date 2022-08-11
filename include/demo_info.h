#ifndef DEMO_INFO_H
#define DEMO_INFO_H

#include "demo_message.h"

enum Game {
    DMOMM,
    HL2_OE,
    PORTAL_3258,
    PORTAL_3420,
    PORTAL_5135,
    PORTAL_1910503,
    PORTAL_2,
    UNKNOWN
};

typedef struct {
    enum Game game;
    uint32_t demo_protocol;
    uint32_t network_protocol;
    uint8_t MSSC;
    bool NE;
    float tickrate;

    const DemoMessageSettings* msg_settings;
    const NetSvcMessageSettings* net_msg_settings;

    bool quick_mode;
} DemoInfo;

extern DemoInfo demo_info;

#endif