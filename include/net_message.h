#ifndef NET_MESSAGES_H
#define NET_MESSAGES_H

#include <stdbool.h>
#include "bits.h"

#define MACRO_ALL_NET_MESSAGES(macro)   \
macro(NetNop)                           \
macro(NetDisconnect)                    \
macro(NetFile)                          \
macro(NetSplitScreenUser)               \
macro(NetTick)                          \
macro(NetStringCmd)                     \
macro(NetSetConVar)                     \
macro(NetSignonState)                   \
macro(SvcServerInfo)                    \
macro(SvcSendTable)                     \
macro(SvcClassInfo)                     \
macro(SvcSetPause)                      \
macro(SvcCreateStringTable)             \
macro(SvcUpdateStringTable)             \
macro(SvcVoiceInit)                     \
macro(SvcVoiceData)                     \
macro(SvcPrint)                         \
macro(SvcSounds)                        \
macro(SvcSetView)                       \
macro(SvcFixAngle)                      \
macro(SvcCrosshairAngle)                \
macro(SvcBspDecal)                      \
macro(SvcSplitScreen)                   \
macro(SvcUserMessage)                   \
macro(SvcEntityMessage)                 \
macro(SvcGameEvent)                     \
macro(SvcPacketEntities)                \
macro(SvcTempEntities)                  \
macro(SvcPrefetch)                      \
macro(SvcMenu)                          \
macro(SvcGameEventList)                 \
macro(SvcGetCvarValue)                  \
macro(SvcCmdKeyValues)                  \
macro(SvcPaintmapData)

#define MACRO_OE_NET_MESSAGES(macro)    \
macro(NetNop)                           \
macro(NetDisconnect)                    \
macro(NetFile)                          \
macro(NetTick)                          \
macro(NetStringCmd)                     \
macro(NetSetConVar)                     \
macro(NetSignonState)                   \
macro(SvcPrint)                         \
macro(SvcServerInfo)                    \
macro(SvcSendTable)                     \
macro(SvcClassInfo)                     \
macro(SvcSetPause)                      \
macro(SvcCreateStringTable)             \
macro(SvcUpdateStringTable)             \
macro(SvcVoiceInit)                     \
macro(SvcVoiceData)                     \
macro(NetInvalid)                       \
macro(SvcSounds)                        \
macro(SvcSetView)                       \
macro(SvcFixAngle)                      \
macro(SvcCrosshairAngle)                \
macro(SvcBspDecal)                      \
macro(NetInvalid)                       \
macro(SvcUserMessage)                   \
macro(SvcEntityMessage)                 \
macro(SvcGameEvent)                     \
macro(SvcPacketEntities)                \
macro(SvcTempEntities)                  \
macro(SvcPrefetch)                      \
macro(SvcMenu)                          \
macro(SvcGameEventList)                 \
macro(SvcGetCvarValue)                  \
macro(SvcCmdKeyValues)

#define MACRO_NE_NET_MESSAGES(macro)    \
macro(NetNop)                           \
macro(NetDisconnect)                    \
macro(NetFile)                          \
macro(NetSplitScreenUser)               \
macro(NetTick)                          \
macro(NetStringCmd)                     \
macro(NetSetConVar)                     \
macro(NetSignonState)                   \
macro(SvcServerInfo)                    \
macro(SvcSendTable)                     \
macro(SvcClassInfo)                     \
macro(SvcSetPause)                      \
macro(SvcCreateStringTable)             \
macro(SvcUpdateStringTable)             \
macro(SvcVoiceInit)                     \
macro(SvcVoiceData)                     \
macro(SvcPrint)                         \
macro(SvcSounds)                        \
macro(SvcSetView)                       \
macro(SvcFixAngle)                      \
macro(SvcCrosshairAngle)                \
macro(SvcBspDecal)                      \
macro(SvcSplitScreen)                   \
macro(SvcUserMessage)                   \
macro(SvcEntityMessage)                 \
macro(SvcGameEvent)                     \
macro(SvcPacketEntities)                \
macro(SvcTempEntities)                  \
macro(SvcPrefetch)                      \
macro(SvcMenu)                          \
macro(SvcGameEventList)                 \
macro(SvcGetCvarValue)                  \
macro(SvcCmdKeyValues)                  \
macro(SvcPaintmapData)

typedef uint8_t NetSvcMessageType;

#define DECL_MSG_IN_ENUM(x) x ## _MSG,
typedef enum {
    NetInvalid_MSG = -1,
    MACRO_ALL_NET_MESSAGES(DECL_MSG_IN_ENUM)
    NET_MSG_COUNT
} NetSvcMessageID;

typedef struct _NetSvcMessage NetSvcMessage;
typedef union _NetSvcMessageData NetSvcMessageData;

typedef bool (*ParseNetSvcMsgFunc)(NetSvcMessageData* thisptr, BitStream* bits);
typedef void (*PrintNetSvcMsgFunc)(const NetSvcMessageData* thisptr, FILE* fp);
typedef void (*FreeNetSvcMsgFunc)(NetSvcMessageData* thisptr);

typedef struct {
    ParseNetSvcMsgFunc parse;
    PrintNetSvcMsgFunc print;
    FreeNetSvcMsgFunc free;
} NetSvcMessageTable;

#define PARSE_FUNC_NAME(type) parse_ ## type
#define PRINT_FUNC_NAME(type) print_ ## type
#define FREE_FUNC_NAME(type) free_ ## type

#define DECL_PTR(type) type* ptr = &thisptr->type ## _message;

#define DECL_MSG_IN_TABLE(x)    \
{                               \
    PARSE_FUNC_NAME(x),         \
    PRINT_FUNC_NAME(x),         \
    FREE_FUNC_NAME(x)           \
},

#define DECL_MSG_IN_NAME(x) #x,

typedef struct {
    const char* names[NET_MSG_COUNT];
    const NetSvcMessageID enum_ids[NET_MSG_COUNT];
    const NetSvcMessageTable func_table[NET_MSG_COUNT];
} NetSvcMessageSettings;

extern const NetSvcMessageSettings oe_net_msg_settings;
extern const NetSvcMessageSettings ne_net_msg_settings;

typedef struct {
    uint8_t empty;
} NetNop;

typedef struct {
    char* text;
} NetDisconnect;

typedef struct {
    uint32_t transfer_id;
    char* file_name;
    uint8_t file_flags;
} NetFile;

typedef struct {
    bool unknown;
} NetSplitScreenUser;

#define NET_TICK_SCALEUP 1e5
typedef struct {
    uint32_t tick;
    uint16_t host_frame_time;
    uint16_t host_frame_time_std_deviation;
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
    uint8_t signon_state;
    uint32_t spawn_count;
    uint32_t num_server_players;
    uint32_t ids_length;
    uint8_t* players_network_ids;
    uint32_t map_name_length;
    char* map_name;
} NetSignonState;

typedef struct {
    uint16_t network_protocol;
    uint32_t server_count;
    bool is_hltv;
    bool is_dedicated;
    uint32_t client_crc;
    uint32_t string_table_crc;
    uint16_t max_class;
    uint8_t map_md5[16];
    uint32_t map_crc;
    uint8_t player_slot;
    uint8_t max_clients;
    float tick_interval;
    char c_os;
    char* game_dir;
    char* map_name;
    char* sky_name;
    char* host_name;
    bool has_replay;
} SvcServerInfo;

typedef struct {
    bool needs_decoder;
    uint16_t length;
    uint8_t* props;
} SvcSendTable;

typedef struct {
    uint32_t class_id;
    char* class_name;
    char* data_table_name;
} ServerClass;

typedef struct {
    uint16_t length;
    bool create_on_client;
    ServerClass* server_classes;
} SvcClassInfo;

typedef struct {
    bool paused;
} SvcSetPause;

typedef struct {
    char* name;
    uint16_t max_entries;
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
    uint16_t num_changed_entries;
    uint32_t length;
    uint8_t* data;
} SvcUpdateStringTable;

typedef struct {
    char* codec;
    uint8_t quality;
    uint32_t unknown;
} SvcVoiceInit;

typedef struct {
    uint8_t client;
    uint8_t proximity;
    uint16_t length;
    uint8_t* data;
} SvcVoiceData;

typedef struct {
    char* message;
} SvcPrint;

typedef struct {
    bool reliable_sound;
    uint8_t size;
    uint16_t length;
    uint8_t* data;
} SvcSounds;

typedef struct {
    uint32_t entity_index;
} SvcSetView;

typedef struct {
    bool relative;
    uint16_t angle[3];
} SvcFixAngle;

typedef struct {
    uint16_t angle[3];
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
    uint32_t unknown;
    uint32_t length;
    uint8_t* data;
} SvcSplitScreen;

typedef struct {
    uint8_t msg_type;
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
    uint8_t num_entries;
    uint32_t length;
    uint8_t* data;
} SvcTempEntities;

typedef struct {
    uint32_t sound_index;
} SvcPrefetch;

typedef struct {
    uint16_t menu_type;
    uint32_t length;
    uint8_t* data;
} SvcMenu;

typedef struct {
    uint32_t event_id;
    char* name;
    uint8_t* keys;
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

typedef NetNop NetInvalid;

#define DECL_MSG_IN_UION(x) x x ## _message;
union _NetSvcMessageData {
    MACRO_ALL_NET_MESSAGES(DECL_MSG_IN_UION)
};

struct _NetSvcMessage {
    NetSvcMessageType type;
    NetSvcMessageData data;
};

#endif