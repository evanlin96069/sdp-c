#include "net_message.h"
#include "demo_info.h"
#include "alloc.h"

#define DECL_NET_PARSE_FUNC(type) static void PARSE_FUNC_NAME(type)(NetSvcMessage* msg, BitStream* bits)
#define DECL_NET_PRINT_FUNC(type) static void  PRINT_FUNC_NAME(type)(const NetSvcMessage* msg, FILE* fp)
#define DECL_NET_FREE_FUNC(type) static void  FREE_FUNC_NAME(type)(NetSvcMessage* msg)

// NetNop
DECL_NET_PARSE_FUNC(NetNop) {}
DECL_NET_PRINT_FUNC(NetNop) {
    fprintf(fp, "\tNetNop\n");
}
DECL_NET_FREE_FUNC(NetNop) {}

// NetDisconnect
DECL_NET_PARSE_FUNC(NetDisconnect) {
    msg->data.NetDisconnect_message.text = bits_read_str(bits);
}
DECL_NET_PRINT_FUNC(NetDisconnect) {
    fprintf(fp, "\tNetDisconnect\n");
    fprintf(fp, "\t\tText: %s\n", msg->data.NetDisconnect_message.text);

}
DECL_NET_FREE_FUNC(NetDisconnect) {
    free(msg->data.NetDisconnect_message.text);
}

// NetFile
DECL_NET_PARSE_FUNC(NetFile) {
    msg->data.NetFile_message.transfer_id = bits_read_le_u32(bits);
    msg->data.NetFile_message.file_name = bits_read_str(bits);
    msg->data.NetFile_message.file_flags = bits_read_bits(demo_info.NE ? 2 : 1, bits);
}
DECL_NET_PRINT_FUNC(NetFile) {
    fprintf(fp, "\tNetFile\n");
    fprintf(fp, "\t\tTransferId: %d\n", msg->data.NetFile_message.transfer_id);
    fprintf(fp, "\t\tFileName: %s\n", msg->data.NetFile_message.file_name);
    fprintf(fp, "\t\tFileFlags: %d\n", msg->data.NetFile_message.file_flags);
}
DECL_NET_FREE_FUNC(NetFile) {
    free(msg->data.NetFile_message.file_name);
}

// NetSplitScreenUser
DECL_NET_PARSE_FUNC(NetSplitScreenUser) {
    msg->data.NetSplitScreenUser_message.unknown = bits_read_one_bit(bits);
}
DECL_NET_PRINT_FUNC(NetSplitScreenUser) {
    fprintf(fp, "\tNetSplitScreenUser\n");
    fprintf(fp, "\t\tUnknown: %s\n", msg->data.NetSplitScreenUser_message.unknown ? "true" : "false");
}
DECL_NET_FREE_FUNC(NetSplitScreenUser) {}

// NetTick
DECL_NET_PARSE_FUNC(NetTick) {
    msg->data.NetTick_message.tick = bits_read_le_u32(bits);
    if (demo_info.game != HL2_OE) {
        msg->data.NetTick_message.host_frame_time = bits_read_le_u16(bits);
        msg->data.NetTick_message.host_frame_time_std_deviation = bits_read_le_u16(bits);
    }
}
DECL_NET_PRINT_FUNC(NetTick) {
    fprintf(fp, "\tNetTick\n");
    fprintf(fp, "\t\tTick: %d\n", msg->data.NetTick_message.tick);
    if (demo_info.game != HL2_OE) {
        fprintf(fp, "\t\tHostFrameTime: %.3f\n", (float)msg->data.NetTick_message.host_frame_time / NET_TICK_SCALEUP);
        fprintf(fp, "\t\tHostFrameTimeStdDev: %.3f\n", (float)msg->data.NetTick_message.host_frame_time_std_deviation / NET_TICK_SCALEUP);
    }
}
DECL_NET_FREE_FUNC(NetTick) {}

// NetStringCmd
DECL_NET_PARSE_FUNC(NetStringCmd) {
    msg->data.NetStringCmd_message.command = bits_read_str(bits);
}
DECL_NET_PRINT_FUNC(NetStringCmd) {
    fprintf(fp, "\tNetStringCmd\n");
    fprintf(fp, "\t\tCommand: %s\n", msg->data.NetStringCmd_message.command);
}
DECL_NET_FREE_FUNC(NetStringCmd) {
    free(msg->data.NetStringCmd_message.command);
}

// NetSetConVar
DECL_NET_PARSE_FUNC(NetSetConVar) {
    uint8_t len = msg->data.NetSetConVar_message.length = bits_read_le_u8(bits);
    msg->data.NetSetConVar_message.cvars = malloc_s(len * sizeof(ConVar));
    for (int i = 0; i < len; i++) {
        msg->data.NetSetConVar_message.cvars[i].name = bits_read_str(bits);
        msg->data.NetSetConVar_message.cvars[i].value = bits_read_str(bits);
    }
}
DECL_NET_PRINT_FUNC(NetSetConVar) {
    uint8_t len = msg->data.NetSetConVar_message.length;
    fprintf(fp, "\tNetSetConVar\n");
    fprintf(fp, "\t\tConVars:\n");
    for (int i = 0; i < len; i++) {
        fprintf(fp, "\t\t\tName: %s\n", msg->data.NetSetConVar_message.cvars[i].name);
        fprintf(fp, "\t\t\tValue: %s\n", msg->data.NetSetConVar_message.cvars[i].value);
    }
}
DECL_NET_FREE_FUNC(NetSetConVar) {
    uint8_t len = msg->data.NetSetConVar_message.length;
    for (int i = 0; i < len; i++) {
        free(msg->data.NetSetConVar_message.cvars[i].name);
        free(msg->data.NetSetConVar_message.cvars[i].value);
    }
    free(msg->data.NetSetConVar_message.cvars);
}

// NetSignonState
DECL_NET_PARSE_FUNC(NetSignonState) {
    msg->data.NetSignonState_message.signon_state = bits_read_le_u8(bits);
    msg->data.NetSignonState_message.spawn_count = bits_read_le_u32(bits);
    if (demo_info.NE) {
        uint8_t len;
        msg->data.NetSignonState_message.num_server_players = bits_read_le_u32(bits);
        len = msg->data.NetSignonState_message.ids_length = bits_read_le_u32(bits);
        if (len > 0) {
            msg->data.NetSignonState_message.players_network_ids = malloc_s(len);
            bits_read_bytes((char*)msg->data.NetSignonState_message.players_network_ids, len, bits);
        }
        len = msg->data.NetSignonState_message.map_name_length = bits_read_le_u32(bits);
        if (len > 0) {
            msg->data.NetSignonState_message.map_name = malloc_s(len);
            bits_read_bytes((char*)msg->data.NetSignonState_message.map_name, len, bits);
        }
    }
}
DECL_NET_PRINT_FUNC(NetSignonState) {
    fprintf(fp, "\tNetSignonState\n");
    fprintf(fp, "\t\tSignonState: %d\n", msg->data.NetSignonState_message.signon_state);
    fprintf(fp, "\t\tSpawnCount: %d\n", msg->data.NetSignonState_message.spawn_count);
    if (demo_info.NE) {
        fprintf(fp, "\t\tNumServerPlayers: %d\n", msg->data.NetSignonState_message.num_server_players);
        uint32_t len = msg->data.NetSignonState_message.ids_length;
        fprintf(fp, "\t\tPlayersNetworkIds:\n\t\t\t[");
        if (len > 0) {
            for (uint32_t i = 0; i < len; i++) {
                fprintf(fp, "%d%s", msg->data.NetSignonState_message.players_network_ids[i], (i == len - 1) ? "]\n" : ", ");
            }
        }
        if (msg->data.NetSignonState_message.map_name_length > 0) {
            fprintf(fp, "\t\tMapName: %s\n", msg->data.NetSignonState_message.map_name);
        }
    }
}
DECL_NET_FREE_FUNC(NetSignonState) {
    if (demo_info.NE) {
        if (msg->data.NetSignonState_message.ids_length > 0) {
            free(msg->data.NetSignonState_message.players_network_ids);
        }
        if (msg->data.NetSignonState_message.map_name_length > 0) {
            free(msg->data.NetSignonState_message.map_name);
        }
    }
}

// SvcServerInfo
DECL_NET_PARSE_FUNC(SvcServerInfo) {}
DECL_NET_PRINT_FUNC(SvcServerInfo) {}
DECL_NET_FREE_FUNC(SvcServerInfo) {}

// SvcSendTable
DECL_NET_PARSE_FUNC(SvcSendTable) {}
DECL_NET_PRINT_FUNC(SvcSendTable) {}
DECL_NET_FREE_FUNC(SvcSendTable) {}

// SvcClassInfo
DECL_NET_PARSE_FUNC(SvcClassInfo) {}
DECL_NET_PRINT_FUNC(SvcClassInfo) {}
DECL_NET_FREE_FUNC(SvcClassInfo) {}

// SvcSetPause
DECL_NET_PARSE_FUNC(SvcSetPause) {}
DECL_NET_PRINT_FUNC(SvcSetPause) {}
DECL_NET_FREE_FUNC(SvcSetPause) {}

// SvcCreateStringTable
DECL_NET_PARSE_FUNC(SvcCreateStringTable) {}
DECL_NET_PRINT_FUNC(SvcCreateStringTable) {}
DECL_NET_FREE_FUNC(SvcCreateStringTable) {}

// SvcUpdateStringTable
DECL_NET_PARSE_FUNC(SvcUpdateStringTable) {}
DECL_NET_PRINT_FUNC(SvcUpdateStringTable) {}
DECL_NET_FREE_FUNC(SvcUpdateStringTable) {}

// SvcVoiceInit
DECL_NET_PARSE_FUNC(SvcVoiceInit) {}
DECL_NET_PRINT_FUNC(SvcVoiceInit) {}
DECL_NET_FREE_FUNC(SvcVoiceInit) {}

// SvcVoiceData
DECL_NET_PARSE_FUNC(SvcVoiceData) {}
DECL_NET_PRINT_FUNC(SvcVoiceData) {}
DECL_NET_FREE_FUNC(SvcVoiceData) {}

// SvcPrint
DECL_NET_PARSE_FUNC(SvcPrint) {}
DECL_NET_PRINT_FUNC(SvcPrint) {}
DECL_NET_FREE_FUNC(SvcPrint) {}

// SvcSounds
DECL_NET_PARSE_FUNC(SvcSounds) {}
DECL_NET_PRINT_FUNC(SvcSounds) {}
DECL_NET_FREE_FUNC(SvcSounds) {}

// SvcSetView
DECL_NET_PARSE_FUNC(SvcSetView) {}
DECL_NET_PRINT_FUNC(SvcSetView) {}
DECL_NET_FREE_FUNC(SvcSetView) {}

// SvcFixAngle
DECL_NET_PARSE_FUNC(SvcFixAngle) {}
DECL_NET_PRINT_FUNC(SvcFixAngle) {}
DECL_NET_FREE_FUNC(SvcFixAngle) {}

// SvcCrosshairAngle
DECL_NET_PARSE_FUNC(SvcCrosshairAngle) {}
DECL_NET_PRINT_FUNC(SvcCrosshairAngle) {}
DECL_NET_FREE_FUNC(SvcCrosshairAngle) {}

// SvcBspDecal
DECL_NET_PARSE_FUNC(SvcBspDecal) {}
DECL_NET_PRINT_FUNC(SvcBspDecal) {}
DECL_NET_FREE_FUNC(SvcBspDecal) {}

// SvcSplitScreen
DECL_NET_PARSE_FUNC(SvcSplitScreen) {}
DECL_NET_PRINT_FUNC(SvcSplitScreen) {}
DECL_NET_FREE_FUNC(SvcSplitScreen) {}

// SvcUserMessage
DECL_NET_PARSE_FUNC(SvcUserMessage) {}
DECL_NET_PRINT_FUNC(SvcUserMessage) {}
DECL_NET_FREE_FUNC(SvcUserMessage) {}

// SvcEntityMessage
DECL_NET_PARSE_FUNC(SvcEntityMessage) {}
DECL_NET_PRINT_FUNC(SvcEntityMessage) {}
DECL_NET_FREE_FUNC(SvcEntityMessage) {}

// SvcGameEvent
DECL_NET_PARSE_FUNC(SvcGameEvent) {}
DECL_NET_PRINT_FUNC(SvcGameEvent) {}
DECL_NET_FREE_FUNC(SvcGameEvent) {}

// SvcPacketEntities
DECL_NET_PARSE_FUNC(SvcPacketEntities) {}
DECL_NET_PRINT_FUNC(SvcPacketEntities) {}
DECL_NET_FREE_FUNC(SvcPacketEntities) {}

// SvcTempEntities
DECL_NET_PARSE_FUNC(SvcTempEntities) {}
DECL_NET_PRINT_FUNC(SvcTempEntities) {}
DECL_NET_FREE_FUNC(SvcTempEntities) {}

// SvcPrefetch
DECL_NET_PARSE_FUNC(SvcPrefetch) {}
DECL_NET_PRINT_FUNC(SvcPrefetch) {}
DECL_NET_FREE_FUNC(SvcPrefetch) {}

// SvcMenu
DECL_NET_PARSE_FUNC(SvcMenu) {}
DECL_NET_PRINT_FUNC(SvcMenu) {}
DECL_NET_FREE_FUNC(SvcMenu) {}

// SvcGameEventList
DECL_NET_PARSE_FUNC(SvcGameEventList) {}
DECL_NET_PRINT_FUNC(SvcGameEventList) {}
DECL_NET_FREE_FUNC(SvcGameEventList) {}

// SvcGetCvarValue
DECL_NET_PARSE_FUNC(SvcGetCvarValue) {}
DECL_NET_PRINT_FUNC(SvcGetCvarValue) {}
DECL_NET_FREE_FUNC(SvcGetCvarValue) {}

// SvcCmdKeyValues
DECL_NET_PARSE_FUNC(SvcCmdKeyValues) {}
DECL_NET_PRINT_FUNC(SvcCmdKeyValues) {}
DECL_NET_FREE_FUNC(SvcCmdKeyValues) {}

// SvcPaintmapData
DECL_NET_PARSE_FUNC(SvcPaintmapData) {}
DECL_NET_PRINT_FUNC(SvcPaintmapData) {}
DECL_NET_FREE_FUNC(SvcPaintmapData) {}

// Invalid
DECL_NET_PARSE_FUNC(NetInvalid) {}
DECL_NET_PRINT_FUNC(NetInvalid) {}
DECL_NET_FREE_FUNC(NetInvalid) {}

// function tables
const NetSvcMessageTable oe_net_massage_table[NET_MSG_COUNT] = {
    MACRO_OE_NET_MESSAGES(DECL_MSG_IN_TABLE)
};

const NetSvcMessageTable ne_net_massage_table[NET_MSG_COUNT] = {
    MACRO_NE_NET_MESSAGES(DECL_MSG_IN_TABLE)
};

const NetSvcMessageID oe_net_massage_ids[NET_MSG_COUNT] = {
    MACRO_OE_NET_MESSAGES(DECL_MSG_IN_ENUM)
};
const NetSvcMessageID ne_net_massage_ids[NET_MSG_COUNT] = {
    MACRO_NE_NET_MESSAGES(DECL_MSG_IN_ENUM)
};