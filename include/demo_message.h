#ifndef DEMO_MESSAGE_H
#define DEMO_MESSAGE_H

#include "net_message.h"
#include "vector.h"

#define MACRO_ALL_MESSAGES(macro)   \
macro(SignOn)                       \
macro(Packet)                       \
macro(SyncTick)                     \
macro(ConsoleCmd)                   \
macro(UserCmd)                      \
macro(DataTables)                   \
macro(Stop)                         \
macro(CustomData)                   \
macro(StringTables)

#define MACRO_PORTAL_3420_MESSAGES(macro)   \
macro(StringTables)                         \
macro(SignOn)                               \
macro(Packet)                               \
macro(SyncTick)                             \
macro(ConsoleCmd)                           \
macro(UserCmd)                              \
macro(DataTables)                           \
macro(Stop)                                 \
macro(Invalid)                              \
macro(Invalid)

#define MACRO_PORTAL_5135_MESSAGES(macro)   \
macro(Invalid)                              \
macro(SignOn)                               \
macro(Packet)                               \
macro(SyncTick)                             \
macro(ConsoleCmd)                           \
macro(UserCmd)                              \
macro(DataTables)                           \
macro(Stop)                                 \
macro(StringTables)                         \
macro(Invalid)

#define MACRO_NE_MESSAGES(macro)    \
macro(Invalid)                      \
macro(SignOn)                       \
macro(Packet)                       \
macro(SyncTick)                     \
macro(ConsoleCmd)                   \
macro(UserCmd)                      \
macro(DataTables)                   \
macro(Stop)                         \
macro(CustomData)                   \
macro(StringTables)

typedef uint8_t DemoMessageType;

typedef enum {
    Invalid_MSG,
    MACRO_ALL_MESSAGES(DECL_MSG_IN_ENUM)
    MESSAGE_COUNT
} DemoMessageID;

typedef struct _DemoMessage DemoMessage;

typedef void (*ParseMessageFunc)(DemoMessage* msg, BitStream* bits);
typedef void (*PrintMessageFunc)(const DemoMessage* msg, FILE* fp);
typedef void (*FreeMessageFunc)(DemoMessage* msg);

typedef struct {
    ParseMessageFunc parse;
    PrintMessageFunc print;
    FreeMessageFunc free;
} DemoMessageTable;

extern const DemoMessageTable portal_3420_massage_table[MESSAGE_COUNT];
extern const DemoMessageTable portal_5135_massage_table[MESSAGE_COUNT];
extern const DemoMessageTable ne_massage_table[MESSAGE_COUNT];

extern const DemoMessageID portal_3420_massage_ids[MESSAGE_COUNT];
extern const DemoMessageID portal_5135_massage_ids[MESSAGE_COUNT];
extern const DemoMessageID ne_massage_ids[MESSAGE_COUNT];

typedef struct {
    int32_t flags;
    float view_origin[3];
    float view_angles[3];
    float local_view_angles[3];
    float view_origin2[3];
    float view_angles2[3];
    float local_view_angles2[3];
} CmdInfo;

typedef VECTOR(NetSvcMessage) Vector_NetSvcMessage;

#define MSSC_MAX 2
typedef struct {
    CmdInfo packet_info[MSSC_MAX];
    uint32_t in_sequence;
    uint32_t out_sequence;
    uint32_t size;
    Vector_NetSvcMessage data;
} SignOn;

typedef SignOn Packet;

typedef struct {
    uint8_t empty;
} SyncTick;

typedef struct {
    uint32_t size;
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
    uint32_t size;
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
    uint32_t size;
    SendTable* send_table;
    ServerClassInfo* server_class_info;
} DataTables;

typedef struct {
    uint8_t* remaining_data;
} Stop;

typedef struct {
    uint32_t unknown;
    uint32_t size;
    uint8_t* data;
} CustomData;

typedef struct {
    bool has_entry_size;
    bool has_entry_data;
    bool has_num_of_client_entries;
    bool has_client_entry_name;
    bool has_client_entry_size;
    bool has_client_entry_data;

    uint8_t num_of_tables;
    char* table_name;
    uint16_t num_of_entries;
    char* entry_name;
    uint16_t entry_size;
    uint8_t* entry_data;
    uint16_t num_of_client_entries;
    char* client_entry_name;
    uint16_t client_entry_size;
    uint8_t* client_entry_data;
} StringTable;

typedef struct {
    uint32_t size;
    StringTable* data;
} StringTables;

struct _DemoMessage {
    DemoMessageType type;
    int32_t tick;
    uint8_t slot;
    union {
        MACRO_ALL_MESSAGES(DECL_MSG_IN_UION)
    } data;
};

#endif