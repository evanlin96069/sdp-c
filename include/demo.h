#ifndef DEMO_H
#define DEMO_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

enum {
    SIGN_ON = 1,
    PACKET,
    SYNC_TICK,
    CONSOLECMD,
    USERCMD,
    DATA_TABLES,
    STOP,
    STRING_TABLES
};

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

typedef struct _DemoMessage DemoMessage;
struct _DemoMessage {
    uint8_t type;
    int32_t tick;
    void* data;
    DemoMessage* next;
};

typedef struct {
    char* path;
    char* file_name;
    DemoHeader header;
    DemoMessage* messages;
} Demo;

typedef struct {
    int32_t flags;
    float view_origin[3];
    float view_angles[3];
    float local_view_angles[3];
    float view_origin2[3];
    float view_angles2[3];
    float local_view_angles2[3];
} CmdInfo;

typedef struct {
    bool has_command_number;
    bool has_tick_count;
    bool has_view_angles_x;
    bool has_view_angles_y;
    bool has_view_angles_z;
    bool has_forward_move;
    bool has_side_move;
    bool has_up_move;
    bool has_buttons;
    bool has_impulse;
    bool has_weapon_select;
    bool has_weapon_subtype;
    bool has_mouse_dx;
    bool has_mouse_dy;

    uint32_t cmd;
    uint32_t command_number;
    uint32_t tick_count;
    float view_angles_x;
    float view_angles_y;
    float view_angles_z;
    float forward_move;
    float side_move;
    float up_move;
    uint32_t buttons;
    uint8_t impulse;
    uint32_t weapon_select;
    uint32_t weapon_subtype;
    uint16_t mouse_dx;
    uint16_t mouse_dy;
} UserCmd;

Demo* new_demo(char* path);
int demo_parse(Demo* demo);
void demo_print_header(const Demo* demo, FILE* fp);
void demo_verbose(const Demo* demo, FILE* fp);
void demo_gen_tas_script(const Demo* demo, FILE* fp);
void demo_free(Demo* demo);

#endif