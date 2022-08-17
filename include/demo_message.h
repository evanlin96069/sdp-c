#ifndef DEMO_MESSAGE_H
#define DEMO_MESSAGE_H

#include "net_message.h"
#include "custom_message.h"
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
typedef union _DemoMessageData DemoMessageData;

typedef bool (*ParseMessageFunc)(DemoMessageData* thisptr, BitStream* bits);
typedef void (*PrintMessageFunc)(const DemoMessageData* thisptr, FILE* fp);
typedef void (*FreeMessageFunc)(DemoMessageData* thisptr);

typedef struct {
    ParseMessageFunc parse;
    PrintMessageFunc print;
    FreeMessageFunc free;
} DemoMessageTable;

typedef struct {
    const char* names[MESSAGE_COUNT];
    const DemoMessageID enum_ids[MESSAGE_COUNT];
    const DemoMessageTable func_table[MESSAGE_COUNT];
} DemoMessageSettings;

extern const DemoMessageSettings portal_3420_msg_settings;
extern const DemoMessageSettings portal_5135_msg_settings;
extern const DemoMessageSettings ne_msg_settings;

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
    OPTIONAL(uint32_t, command_number);
    OPTIONAL(uint32_t, tick_count);
    OPTIONAL(float, view_angles_x);
    OPTIONAL(float, view_angles_y);
    OPTIONAL(float, view_angles_z);
    OPTIONAL(float, forward_move);
    OPTIONAL(float, side_move);
    OPTIONAL(float, up_move);
    OPTIONAL(uint32_t, buttons);
    OPTIONAL(uint8_t, impulse);
    OPTIONAL(uint32_t, weapon_select);
    OPTIONAL(uint32_t, weapon_subtype);
    OPTIONAL(int16_t, mouse_dx);
    OPTIONAL(int16_t, mouse_dy);

    // DMoMM
    uint8_t mm_unknown_weapon_flags;
    OPTIONAL(bool, unknown_b_1);
    bool unknown_b_2;
    OPTIONAL(uint16_t, unknown_u11);
    OPTIONAL(int32_t, mm_move_item_from_slot);
    OPTIONAL(int32_t, mm_move_item_to_slot);
    OPTIONAL(float, mm_stealth);
    OPTIONAL(uint8_t, mm_use_item_id);
    bool mm_unknown_item_flag;
    OPTIONAL(int8_t, unknown_i6);
    OPTIONAL(int8_t, mm_upgrade_skill_type);
    OPTIONAL(int16_t, unknown_i16);
    OPTIONAL(float, mm_lean_move);
    OPTIONAL(bool, mm_sprint);
    OPTIONAL(bool, mm_unknown_action_2);
    OPTIONAL(bool, mm_kick);
    OPTIONAL(bool, mm_unknown_action_4);
    OPTIONAL(bool, mm_unknown_action_5);
    OPTIONAL(bool, mm_shwo_charsheet);
    OPTIONAL(bool, mm_unknown_action_7);
    OPTIONAL(bool, mm_show_inventory_belt);
    OPTIONAL(bool, mm_show_inventory_belt_select);
    OPTIONAL(bool, mm_hide_inventory_belt_select);
    OPTIONAL(bool, mm_show_objectives);
    OPTIONAL(bool, mm_hide_objectives);
    OPTIONAL(int32_t, mm_exit_book_id);
    OPTIONAL(bool, mm_xana);
} UserCmdInfo;

typedef struct {
    uint32_t cmd;
    uint32_t size;
    UserCmdInfo data;
} UserCmd;

enum SendPropType {
    SEND_PROP_INT,
    SEND_PROP_FLOAT,
    SEND_PROP_VECTOR3,
    SEND_PROP_VECTOR2,
    SEND_PROP_STRING,
    SEND_PROP_ARRAY,
    SEND_PROP_DATATABLE,
};

typedef struct {
    enum SendPropType send_prop_type;
    char* send_prop_name;
    uint32_t send_prop_flags;
    uint8_t priority;
    char* exclude_dt_name;
    float low_value;
    float high_value;
    uint32_t num_bits;
    uint32_t num_element;
} SendProp;

typedef VECTOR(SendProp) Vector_SendProp;

typedef struct {
    bool needs_decoder;
    char* net_table_name;
    uint32_t num_of_props;
    Vector_SendProp send_props;
} SendTable;

typedef struct {
    uint16_t num_of_classes;
    uint16_t class_id;
    char* class_name;
    char* data_table_name;
} ServerClassInfo;

typedef VECTOR(SendTable) Vector_SendTable;
typedef VECTOR(ServerClassInfo) Vector_ServerClassInfo;

typedef struct {
    uint32_t size;
    Vector_SendTable send_tables;
    Vector_ServerClassInfo server_class_info;
} DataTables;

typedef struct {
    size_t remaining_bytes;
} Stop;

typedef struct {
    uint32_t type;
    uint32_t size;
    union {
        RadialMouseMenuCallback RadialMouseMenuCallback_message;
    } data;
} CustomData;

typedef struct {
    uint8_t num_of_tables;
    char* table_name;
    uint16_t num_of_entries;
    char* entry_name;
    OPTIONAL(uint16_t, entry_size);
    OPTIONAL(uint8_t*, entry_data);
    OPTIONAL(uint16_t, num_of_client_entries);
    OPTIONAL(char*, client_entry_name);
    OPTIONAL(uint16_t, client_entry_size);
    OPTIONAL(uint8_t*, client_entry_data);
} StringTable;

typedef struct {
    uint32_t size;
    StringTable* data;
} StringTables;

typedef Stop Invalid;

union _DemoMessageData {
    MACRO_ALL_MESSAGES(DECL_MSG_IN_UION)
};

struct _DemoMessage {
    DemoMessageType type;
    int32_t tick;
    uint8_t slot;
    DemoMessageData data;
};

#endif