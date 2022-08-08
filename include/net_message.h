#ifndef NET_MESSAGES_H
#define NET_MESSAGES_H

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bits.h"

#define MACRO_NE_MESSAGES(macro)    \
macro(NetNop)                       \
macro(NetDisconnect)                \
macro(NetFile)                      \
macro(NetSplitScreenUser)           \
macro(NetTick)                      \
macro(NetStringCmd)                 \
macro(NetSetConVar)                 \
macro(NetSignonState)               \
macro(SvcServerInfo)                \
macro(SvcSendTable)                 \
macro(SvcClassInfo)                 \
macro(SvcSetPause)                  \
macro(SvcCreateStringTable)         \
macro(SvcUpdateStringTable)         \
macro(SvcVoiceInit)                 \
macro(SvcVoiceData)                 \
macro(SvcPrint)                     \
macro(SvcSounds)                    \
macro(SvcSetView)                   \
macro(SvcFixAngle)                  \
macro(SvcCrosshairAngle)            \
macro(SvcBspDecal)                  \
macro(SvcSplitScreen)               \
macro(SvcUserMessage)               \
macro(SvcEntityMessage)             \
macro(SvcGameEvent)                 \
macro(SvcPacketEntities)            \
macro(SvcTempEntities)              \
macro(SvcPrefetch)                  \
macro(SvcMenu)                      \
macro(SvcGameEventList)             \
macro(SvcGetCvarValue)              \
macro(SvcCmdKeyValues)              \
macro(SvcPaintmapData)

#define MACRO_OE_MESSAGES(macro)    \
macro(NetNop)                       \
macro(NetDisconnect)                \
macro(NetFile)                      \
macro(NetTick)                      \
macro(NetStringCmd)                 \
macro(NetSetConVar)                 \
macro(NetSignonState)               \
macro(SvcPrint)                     \
macro(SvcServerInfo)                \
macro(SvcSendTable)                 \
macro(SvcClassInfo)                 \
macro(SvcSetPause)                  \
macro(SvcCreateStringTable)         \
macro(SvcUpdateStringTable)         \
macro(SvcVoiceInit)                 \
macro(SvcVoiceData)                 \
macro(Invalid1)                     \
macro(SvcSounds)                    \
macro(SvcSetView)                   \
macro(SvcFixAngle)                  \
macro(SvcCrosshairAngle)            \
macro(SvcBspDecal)                  \
macro(Invalid2)                     \
macro(SvcUserMessage)               \
macro(SvcEntityMessage)             \
macro(SvcGameEvent)                 \
macro(SvcPacketEntities)            \
macro(SvcTempEntities)              \
macro(SvcPrefetch)                  \
macro(SvcMenu)                      \
macro(SvcGameEventList)             \
macro(SvcGetCvarValue)              \
macro(SvcCmdKeyValues)

typedef uint8_t NetSvcMessageType;

#define DECL_OE_MSG_IN_ENUM(x) OE_ ## x,
enum {
    MACRO_OE_MESSAGES(DECL_OE_MSG_IN_ENUM)
    OE_MSG_COUNT
};

#define DECL_NE_MSG_IN_ENUM(x) NE_ ## x,
enum {
    MACRO_NE_MESSAGES(DECL_NE_MSG_IN_ENUM)
    NE_MSG_COUNT
};

typedef struct _NetSvcMessage NetSvcMessage;

typedef void (*ParseNetSvcMsgFunc)(NetSvcMessage* msg, BitStream* bits);
typedef void (*PrintNetSvcMsgFunc)(const NetSvcMessage* msg, FILE* fp);
typedef void (*FreeNetSvcMsgFunc)(NetSvcMessage* msg);

extern ParseNetSvcMsgFunc parse_oe_net_msg_func[OE_MSG_COUNT];
extern PrintNetSvcMsgFunc print_oe_net_msg_func[OE_MSG_COUNT];
extern FreeNetSvcMsgFunc free_oe_net_msg_func[OE_MSG_COUNT];

extern ParseNetSvcMsgFunc parse_ne_net_msg_func[NE_MSG_COUNT];
extern PrintNetSvcMsgFunc print_ne_net_msg_func[NE_MSG_COUNT];
extern FreeNetSvcMsgFunc free_ne_net_msg_func[NE_MSG_COUNT];

typedef struct {
    uint8_t empty;
} NetNop;

typedef struct {
    char* text;
} NetDisconnect;

typedef struct {
    uint32_t transfer_id;
    char* file_name;
    bool file_requested;
} NetFile;

typedef struct {
    bool unknown;
} NetSplitScreenUser;

#define NET_TICK_SCALEUP 1e5
typedef struct {
    uint32_t tick;
    uint32_t host_frame_time;
    uint32_t host_frame_time_std_deviation;
} NetTick;

typedef struct {
    char* command;
} NetStringCmd;

typedef struct {
    char* name;
    char* value;
} ConVar;

typedef struct {
    uint32_t length;
    ConVar* cvars;
} NetSetConVar;

typedef struct {
    uint32_t signon_state;
    uint32_t spawn_count;
} NetSignonState;

typedef struct {
    uint32_t protocol;
    uint32_t server_count;
    bool is_hltv;
    bool is_dedicated;
    uint32_t client_crc;
    uint32_t max_class;
    uint32_t map_crc;
    uint32_t player_slot;
    uint32_t max_clients;
    uint32_t unk[3];
    float tick_interval;
    char c_os;
    char* game_dir;
    char* map_name;
    char* sky_name;
    char* host_name;
} SvcServerInfo;

typedef struct {
    bool needs_decoder;
    uint32_t length;
    uint8_t* props;
} SvcSendTable;

typedef struct {
    uint16_t class_id;
    char* class_name;
    char* data_table_name;
} ServerClass;

typedef struct {
    uint32_t length;
    bool create_on_client;
    ServerClass* server_classes;
} SvcClassInfo;

typedef struct {
    bool paused;
} SvcSetPause;

typedef struct {
    char* name;
    uint32_t max_entries;
    uint32_t num_entries;
    uint32_t length;
    bool user_data_fixed_size;
    uint32_t user_data_size;
    uint32_t user_data_size_bits;
    uint32_t flags;
    uint8_t* string_data;
} SvcCreateStringTable;

typedef struct {
    uint32_t table_id;
    bool has_num_changed_entries;
    uint32_t num_changed_entries;
    uint32_t length;
    uint8_t* data;
} SvcUpdateStringTable;

typedef struct {
    char* codec;
    uint32_t quality;
    float unk;
} SvcVoiceInit;

typedef struct {
    uint32_t client;
    uint32_t proximity;
    uint32_t length;
    uint8_t* data;
} SvcVoiceData;

typedef struct {
    char* message;
} SvcPrint;

typedef struct {
    bool reliable_sound;
    uint32_t size;
    uint32_t length;
    uint8_t* data;
} SvcSounds;

typedef struct {
    uint32_t entity_index;
} SvcSetView;

typedef struct {
    bool relative;
    uint32_t angle[3];
} SvcFixAngle;

typedef struct {
    uint32_t angle[3];
} SvcCrosshairAngle;

typedef struct {
    bool has_pos_x;
    bool has_pos_y;
    bool has_pos_z;
    float pos_x;
    float pos_y;
    float pos_z;
    uint32_t decal_texture_index;
    uint32_t entity_index;
    uint32_t model_index;
    bool low_priority;
} SvcBspDecal;

typedef struct {
    uint32_t Unk;
    uint32_t length;
    uint8_t* data;
} SvcSplitScreen;

typedef struct {
    uint32_t msg_type;
    uint32_t length;
    uint8_t* data;
} SvcUserMessage;

typedef struct {
    uint32_t entity_index;
    uint32_t class_id;
    uint32_t length;
    uint8_t* data;
} SvcEntityMessage;

typedef struct {
    uint32_t event_id;
    uint8_t* data;
} GameEvent;

typedef struct {
    uint32_t length;
    GameEvent* data;
} SvcGameEvent;

typedef struct {
    uint32_t max_entries;
    bool is_delta;
    uint32_t delta_from;
    bool base_line;
    uint32_t updated_entries;
    uint32_t length;
    bool update_baseline;
    uint8_t* data;
} SvcPacketEntities;

typedef struct {
    uint32_t num_entries;
    uint32_t length;
    uint8_t* data;
} SvcTempEntities;

typedef struct {
    uint32_t sound_index;
} SvcPrefetch;

typedef struct {
    uint32_t menu_type;
    uint32_t length;
    uint8_t* data;
} SvcMenu;

typedef struct {
    uint32_t event_id;
    char* name;
    // TODO: hash map here
} GameEventDescriptor;

typedef struct {
    uint32_t events;
    uint32_t length;
    GameEventDescriptor* data;
} SvcGameEventList;

typedef struct {
    char cookie[4];
    char* cvar_name;
} SvcGetCvarValue;

typedef struct {
    uint32_t length;
    uint8_t* data;
} SvcCmdKeyValues;

typedef struct {
    uint32_t length;
    uint8_t* data;
} SvcPaintmapData;

#define DECL_MSG_IN_UION(x) x x ## _message;
struct _NetSvcMessage {
    NetSvcMessageType type;
    union {
        MACRO_NE_MESSAGES(DECL_MSG_IN_UION)
    } data;
};

#endif