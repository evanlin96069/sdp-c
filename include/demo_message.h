#ifndef DEMO_MESSAGE_H
#define DEMO_MESSAGE_H

#include <stddef.h>
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
    int32_t flags;
    float view_origin[3];
    float view_angles[3];
    float local_view_angles[3];
    float view_origin2[3];
    float view_angles2[3];
    float local_view_angles2[3];
} CmdInfo;

typedef struct {
    CmdInfo packet_info;
    uint32_t in_sequence;
    uint32_t out_sequence;
    size_t size;
    // NET/SVC-Message[]
    void* data;
} Packet;

typedef struct {
    size_t size;
    char* data;
} ConsoleCmd;

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
} UserCmdInfo;

typedef struct {
    uint32_t cmd;
    size_t size;
    UserCmdInfo data;
} UserCmd;

typedef struct {
    bool needs_decoder;
    char* net_table_name;
    uint32_t num_of_props;
    uint32_t send_prop_type;
    char* send_prop_name;
    uint32_t send_prop_flags;
} SendTable;

typedef struct {
    uint16_t num_of_classes;
    uint16_t class_id;
    char* class_name;
    char* data_table_name;
} ServerClassInfo;

typedef struct {
    size_t size;
    SendTable* send_table;
    ServerClassInfo* server_class_info;
} DataTables;

typedef struct {
    uint32_t num_of_tables;
    char* table_name;
    uint16_t num_of_entries;
    char* entry_name;
    uint16_t entry_size;
    void* entry_data;
    uint16_t num_of_client_entries;
    char* client_entry_name;
    uint16_t client_entry_size;
    void* client_entry_data;
} StringTable;

typedef struct {
    size_t size;
    StringTable* data;
} StringTables;

typedef struct _DemoMessage DemoMessage;
struct _DemoMessage {
    uint8_t type;
    int32_t tick;
    union {
        Packet packet;
        ConsoleCmd console_cmd;
        UserCmd user_cmd;
        DataTables data_tables;
        StringTables string_tables;
    } data;
    DemoMessage* next;
};

#endif