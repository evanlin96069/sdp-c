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

typedef struct {
    char* path;
    char* file_name;
    DemoHeader header;
    DemoMessage* messages;
} Demo;

Demo* new_demo(char* path);
int demo_parse(Demo* demo, bool quick_mode);
void demo_print_header(const Demo* demo, FILE* fp);
void demo_verbose(const Demo* demo, FILE* fp);
void demo_gen_tas_script(const Demo* demo, FILE* fp);
void demo_free(Demo* demo);

#endif