#ifndef DEMO_H
#define DEMO_H

#include "demo_message.h"

typedef struct {
    char demo_file_stamp[8];
    uint32_t demo_protocol;
    uint32_t network_protocol;
    char server_name[260];
    char client_name[260];
    char map_name[260];
    char game_dir[260];
    float play_back_time;
    uint32_t play_back_ticks;
    uint32_t play_back_frames;
    uint32_t sign_on_length;
} DemoHeader;

typedef VECTOR(DemoMessage) Vector_DemoMessage;

typedef struct {
    char* path;
    char* file_name;
    DemoHeader header;
    Vector_DemoMessage messages;
} Demo;

enum Game {
    DMOMM,
    HL2_OE,
    PORTAL_3258,
    PORTAL_3420,
    PORTAL_5135,
    PORTAL_1910503,
    PORTAL_2,
    GAME_UNKNOWN,
    GAME_COUNT
};

typedef enum {
    MEASURED_SUCCESS,
    MEASURED_ERROR,
    NOT_MEASURED
} DemoTimeState;

typedef struct {
    DemoTimeState state;
    enum Game game;
    int ticks;
    float tick_interval;
} DemoTime;

extern const char* game_names[GAME_COUNT];

Demo* new_demo(char* path);
DemoTime demo_parse(Demo* demo, uint8_t parse_level, bool debug_mode);
void demo_print_header(const Demo* demo, FILE* fp);
void demo_verbose(const Demo* demo, FILE* fp);
void demo_gen_tas_script(const Demo* demo, FILE* fp);
void demo_free(Demo* demo);

#endif