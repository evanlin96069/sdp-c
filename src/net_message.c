#include "net_message.h"
#include "demo_info.h"
#include "utils/alloc.h"
#include "utils/indent_writer.h"

#define DECL_NET_PARSE_FUNC(type) static bool PARSE_FUNC_NAME(type)(NetSvcMessageData* thisptr, BitStream* bits)
#define DECL_NET_PRINT_FUNC(type) static void  PRINT_FUNC_NAME(type)(const NetSvcMessageData* thisptr)
#define DECL_NET_FREE_FUNC(type) static void  FREE_FUNC_NAME(type)(NetSvcMessageData* thisptr)

static uint32_t highest_bit_index(uint32_t x) {
    uint32_t result = 0;
    while (x >>= 1) result++;
    return result;
}

// NetNop
DECL_NET_PARSE_FUNC(NetNop) {
    return true;
}
DECL_NET_PRINT_FUNC(NetNop) {}
DECL_NET_FREE_FUNC(NetNop) {}

// NetDisconnect
DECL_NET_PARSE_FUNC(NetDisconnect) {
    DECL_PTR(NetDisconnect);
    ptr->text = bits_read_str(bits);
    return true;
}
DECL_NET_PRINT_FUNC(NetDisconnect) {
    const DECL_PTR(NetDisconnect);
    write_string("Text", ptr->text);
}
DECL_NET_FREE_FUNC(NetDisconnect) {
    DECL_PTR(NetDisconnect);
    free(ptr->text);
}

// NetFile
DECL_NET_PARSE_FUNC(NetFile) {
    DECL_PTR(NetFile);
    ptr->transfer_id = bits_read_le_u32(bits);
    ptr->file_name = bits_read_str(bits);
    ptr->file_flags = bits_read_bits((demo_info.demo_protocol >= 4) ? 2 : 1, bits);
    return true;
}
DECL_NET_PRINT_FUNC(NetFile) {
    const DECL_PTR(NetFile);
    write_int("TransferId", ptr->transfer_id);
    write_string("FileName", ptr->file_name);
    write_int("FileFlags", ptr->file_flags);
}
DECL_NET_FREE_FUNC(NetFile) {
    DECL_PTR(NetFile);
    free(ptr->file_name);
}

// NetSplitScreenUser
DECL_NET_PARSE_FUNC(NetSplitScreenUser) {
    DECL_PTR(NetSplitScreenUser);
    ptr->unknown = bits_read_one_bit(bits);
    return true;
}
DECL_NET_PRINT_FUNC(NetSplitScreenUser) {
    const DECL_PTR(NetSplitScreenUser);
    write_bool("Unknown", ptr->unknown);
}
DECL_NET_FREE_FUNC(NetSplitScreenUser) {}

// NetTick
DECL_NET_PARSE_FUNC(NetTick) {
    DECL_PTR(NetTick);
    ptr->tick = bits_read_le_u32(bits);
    if (demo_info.network_protocol > 10) {
        ptr->host_frame_time = bits_read_le_u16(bits);
        ptr->host_frame_time_std_deviation = bits_read_le_u16(bits);
    }
    return true;
}
DECL_NET_PRINT_FUNC(NetTick) {
    const DECL_PTR(NetTick);
    write_int("Tick", ptr->tick);
    if (demo_info.network_protocol > 10) {
        write_float("HostFrameTime", (float)ptr->host_frame_time / NET_TICK_SCALEUP);
        write_float("HostFrameTimeStdDev", (float)ptr->host_frame_time_std_deviation / NET_TICK_SCALEUP);
    }
}
DECL_NET_FREE_FUNC(NetTick) {}

// NetStringCmd
DECL_NET_PARSE_FUNC(NetStringCmd) {
    DECL_PTR(NetStringCmd);
    ptr->command = bits_read_str(bits);
    return true;
}
DECL_NET_PRINT_FUNC(NetStringCmd) {
    const DECL_PTR(NetStringCmd);
    write_string("Command", ptr->command);
}
DECL_NET_FREE_FUNC(NetStringCmd) {
    DECL_PTR(NetStringCmd);
    free(ptr->command);
}

// NetSetConVar
DECL_NET_PARSE_FUNC(NetSetConVar) {
    DECL_PTR(NetSetConVar);
    uint8_t len = ptr->length = bits_read_le_u8(bits);
    ptr->cvars = malloc_s(len * sizeof(ConVar));
    for (int i = 0; i < len; i++) {
        ptr->cvars[i].name = bits_read_str(bits);
        ptr->cvars[i].value = bits_read_str(bits);
    }
    return true;
}
DECL_NET_PRINT_FUNC(NetSetConVar) {
    const DECL_PTR(NetSetConVar);
    uint8_t len = ptr->length;
    write_line("ConVars:\n");
    for (int i = 0; i < len; i++) {
        g_writer.indent++;
        write_string("Name", ptr->cvars[i].name);
        write_string("Value", ptr->cvars[i].value);
        g_writer.indent--;
    }
}
DECL_NET_FREE_FUNC(NetSetConVar) {
    DECL_PTR(NetSetConVar);
    uint8_t len = ptr->length;
    for (int i = 0; i < len; i++) {
        free(ptr->cvars[i].name);
        free(ptr->cvars[i].value);
    }
    free(ptr->cvars);
}

// NetSignonState
DECL_NET_PARSE_FUNC(NetSignonState) {
    DECL_PTR(NetSignonState);
    ptr->signon_state = bits_read_le_u8(bits);
    ptr->spawn_count = bits_read_le_u32(bits);
    if (demo_info.demo_protocol >= 4) {
        uint8_t len;
        ptr->num_server_players = bits_read_le_u32(bits);
        len = ptr->ids_length = bits_read_le_u32(bits);
        if (len > 0) {
            ptr->players_network_ids = malloc_s(len);
            bits_read_bytes((char*)ptr->players_network_ids, len, bits);
        }
        len = ptr->map_name_length = bits_read_le_u32(bits);
        if (len > 0) {
            ptr->map_name = malloc_s(len);
            bits_read_bytes((char*)ptr->map_name, len, bits);
        }
    }
    return true;
}
DECL_NET_PRINT_FUNC(NetSignonState) {
    const DECL_PTR(NetSignonState);
    write_int("SignonState", ptr->signon_state);
    write_int("SpawnCount", ptr->spawn_count);
    if (demo_info.demo_protocol >= 4) {
        write_int("NumServerPlayers", ptr->num_server_players);
        uint32_t len = ptr->ids_length;
        write_line("\t\t\tPlayersNetworkIds: [ ");
        for (uint32_t i = 0; i < len; i++) {
            if (i != 0) {
                fprintf(g_writer.fp, ", ");
            }
            fprintf(g_writer.fp, "%d", ptr->players_network_ids[i]);
        }
        fprintf(g_writer.fp, "]\n");
        if (ptr->map_name_length > 0) {
            write_string("MapName", ptr->map_name);
        }
    }
}
DECL_NET_FREE_FUNC(NetSignonState) {
    DECL_PTR(NetSignonState);
    if (demo_info.demo_protocol >= 4) {
        if (ptr->ids_length > 0) {
            free(ptr->players_network_ids);
        }
        if (ptr->map_name_length > 0) {
            free(ptr->map_name);
        }
    }
}

// SvcServerInfo
DECL_NET_PARSE_FUNC(SvcServerInfo) {
    DECL_PTR(SvcServerInfo);
    ptr->network_protocol = bits_read_le_u16(bits);
    ptr->server_count = bits_read_le_u32(bits);
    ptr->is_hltv = bits_read_one_bit(bits);
    ptr->is_dedicated = bits_read_one_bit(bits);
    ptr->client_crc = bits_read_le_u32(bits);
    if (demo_info.demo_protocol >= 4) {
        ptr->string_table_crc = bits_read_le_u32(bits);
    }
    ptr->max_class = bits_read_le_u16(bits);

    if (demo_info.network_protocol == 24) {
        bits_read_bytes((char*)ptr->map_md5, 16, bits);
    }
    else {
        ptr->map_crc = bits_read_le_u32(bits);
    }

    ptr->player_slot = bits_read_le_u8(bits);
    ptr->max_clients = bits_read_le_u8(bits);
    ptr->tick_interval = bits_read_le_f32(bits);
    ptr->c_os = bits_read_le_u8(bits);
    ptr->game_dir = bits_read_str(bits);
    ptr->map_name = bits_read_str(bits);
    ptr->sky_name = bits_read_str(bits);
    ptr->host_name = bits_read_str(bits);
    if (demo_info.network_protocol == 24) {
        ptr->has_replay = bits_read_one_bit(bits);
    }

    if (!demo_info.has_tick_interval) {
        demo_info.tick_interval = ptr->tick_interval;
        demo_info.has_tick_interval = true;
    }

    return true;
}
DECL_NET_PRINT_FUNC(SvcServerInfo) {
    const DECL_PTR(SvcServerInfo);
    write_int("NetworkProtocol", ptr->network_protocol);
    write_int("ServerCount", ptr->server_count);
    write_bool("IsHltv", ptr->is_hltv);
    write_bool("IsDedicated", ptr->is_dedicated);
    write_int("ClientCrc", ptr->client_crc);
    if (demo_info.demo_protocol >= 4) {
        write_int("StringTableCrc", ptr->string_table_crc);
    }
    write_int("MaxClass", ptr->max_class);

    if (demo_info.network_protocol == 24) {
        const uint32_t* md5 = (uint32_t*)&ptr->map_md5;
        write_line("MapMd5: %x%x%x%x\n", md5[0], md5[1], md5[2], md5[3]);
    }
    else {
        write_int("MapCrc", ptr->map_crc);
    }

    write_int("PlayerSlot", ptr->player_slot);
    write_int("MaxClients", ptr->max_clients);
    write_float("TickInterval", ptr->tick_interval);
    write_line("COs: %c\n", ptr->c_os);
    write_string("GameDir", ptr->game_dir);
    write_string("MapName", ptr->map_name);
    write_string("SkyName", ptr->sky_name);
    write_string("HostName", ptr->host_name);
    if (demo_info.network_protocol == 24) {
        write_bool("HasReplay", ptr->has_replay);
    }
}
DECL_NET_FREE_FUNC(SvcServerInfo) {
    DECL_PTR(SvcServerInfo);
    free(ptr->game_dir);
    free(ptr->map_name);
    free(ptr->sky_name);
    free(ptr->host_name);
}

// SvcSendTable
DECL_NET_PARSE_FUNC(SvcSendTable) {
    DECL_PTR(SvcSendTable);
    ptr->needs_decoder = bits_read_one_bit(bits);
    uint32_t len = ptr->length = bits_read_le_u16(bits);
    // props parsing not implemented
    ptr->props = bits_read_bits_arr(len, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcSendTable) {
    const DECL_PTR(SvcSendTable);
    write_bool("NeedDecoder", ptr->needs_decoder);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcSendTable) {
    DECL_PTR(SvcSendTable);
    free(ptr->props);
}

// SvcClassInfo
DECL_NET_PARSE_FUNC(SvcClassInfo) {
    DECL_PTR(SvcClassInfo);
    uint32_t len = ptr->length = bits_read_le_u16(bits);
    ptr->create_on_client = bits_read_one_bit(bits);
    uint32_t class_id_bits = highest_bit_index(len) + 1;
    ServerClass* classes = malloc_s(len * sizeof(ServerClass));
    if (!ptr->create_on_client) {
        for (uint32_t i = 0; i < len; i++) {
            classes[i].class_id = bits_read_bits(class_id_bits, bits);
            classes[i].class_name = bits_read_str(bits);
            classes[i].data_table_name = bits_read_str(bits);
        }
    }
    ptr->server_classes = classes;
    return true;
}
DECL_NET_PRINT_FUNC(SvcClassInfo) {
    const DECL_PTR(SvcClassInfo);
    uint32_t len = ptr->length;
    write_bool("CreateOnClient", ptr->create_on_client);
    const ServerClass* classes = ptr->server_classes;
    if (!ptr->create_on_client) {
        write_line("ServerClasses\n");
        for (uint32_t i = 0; i < len; i++) {
            g_writer.indent++;
            write_int("ClassId", classes[i].class_id);
            write_string("ClassName", classes[i].class_name);
            write_string("DataTableName", classes[i].data_table_name);
            g_writer.indent--;
        }
    }
}
DECL_NET_FREE_FUNC(SvcClassInfo) {
    DECL_PTR(SvcClassInfo);
    uint32_t len = ptr->length;
    if (!ptr->create_on_client) {
        for (uint32_t i = 0; i < len; i++) {
            free(ptr->server_classes[i].class_name);
            free(ptr->server_classes[i].data_table_name);
        }
    }
    free(ptr->server_classes);
}

// SvcSetPause
DECL_NET_PARSE_FUNC(SvcSetPause) {
    DECL_PTR(SvcSetPause);
    ptr->paused = bits_read_one_bit(bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcSetPause) {
    const DECL_PTR(SvcSetPause);
    write_bool("Paused", ptr->paused);
}
DECL_NET_FREE_FUNC(SvcSetPause) {}

// SvcCreateStringTable
DECL_NET_PARSE_FUNC(SvcCreateStringTable) {
    DECL_PTR(SvcCreateStringTable);
    ptr->name = bits_read_str(bits);
    ptr->max_entries = bits_read_le_u16(bits);
    uint32_t num_entries_bits = highest_bit_index(ptr->max_entries) + 1;
    ptr->num_entries = bits_read_bits(num_entries_bits, bits);

    if (demo_info.network_protocol == 24) {
        ptr->length = bits_read_varuint32(bits);
    }
    else {
        ptr->length = bits_read_bits(20, bits);
    }

    ptr->user_data_fixed_size = bits_read_one_bit(bits);
    ptr->user_data_size = ptr->user_data_fixed_size ? bits_read_bits(12, bits) : 0;
    ptr->user_data_size_bits = ptr->user_data_fixed_size ? bits_read_bits(4, bits) : 0;

    if (demo_info.network_protocol >= 15) {
        ptr->flags = bits_read_bits((demo_info.demo_protocol >= 4) ? 2 : 1, bits);
    }
    else {
        ptr->flags = 0;
    }

    // string_data parsing not implemented
    ptr->string_data = (uint8_t*)bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcCreateStringTable) {
    const DECL_PTR(SvcCreateStringTable);
    write_string("Name", ptr->name);
    write_int("MaxEntries", ptr->max_entries);
    write_int("NumEntries", ptr->num_entries);
    write_int("Length", ptr->length);
    write_bool("UserDataFixedSize", ptr->user_data_fixed_size);
    if (ptr->user_data_fixed_size) {
        write_int("UserDataSize", ptr->user_data_size);
        write_int("UserDataSizeBits", ptr->user_data_size_bits);
    }
    if (demo_info.network_protocol >= 15) {
        write_int("Flags", ptr->flags);
    }
}
DECL_NET_FREE_FUNC(SvcCreateStringTable) {
    DECL_PTR(SvcCreateStringTable);
    free(ptr->name);
    free(ptr->string_data);
}

// SvcUpdateStringTable
DECL_NET_PARSE_FUNC(SvcUpdateStringTable) {
    DECL_PTR(SvcUpdateStringTable);
    ptr->table_id = bits_read_bits((demo_info.network_protocol <= 6) ? 4 : 5, bits);
    ptr->has_num_changed_entries = bits_read_one_bit(bits);
    if (ptr->has_num_changed_entries) {
        ptr->num_changed_entries = bits_read_le_u16(bits);
    }

    if (demo_info.network_protocol <= 7) {
        ptr->length = bits_read_bits(16, bits);
    }
    else {
        ptr->length = bits_read_bits(20, bits);
    }

    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcUpdateStringTable) {
    const DECL_PTR(SvcUpdateStringTable);
    write_int("TableId", ptr->table_id);
    if (ptr->has_num_changed_entries) {
        write_int("NumChangedEntries", ptr->num_changed_entries);
    }
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcUpdateStringTable) {
    DECL_PTR(SvcUpdateStringTable);
    free(ptr->data);
}

// SvcVoiceInit
DECL_NET_PARSE_FUNC(SvcVoiceInit) {
    DECL_PTR(SvcVoiceInit);
    ptr->codec = bits_read_str(bits);
    ptr->quality = bits_read_le_u8(bits);
    if (ptr->quality == 255) {
        if (demo_info.network_protocol == 24) {
            ptr->unknown = bits_read_le_u16(bits);
        }
        else if (demo_info.demo_protocol == 4) {
            ptr->unknown = bits_read_le_u32(bits);
        }
    }
    return true;
}
DECL_NET_PRINT_FUNC(SvcVoiceInit) {
    const DECL_PTR(SvcVoiceInit);
    write_string("Codec", ptr->codec);
    write_int("Quality", ptr->quality);
    if (ptr->quality == 255) {
        if (demo_info.network_protocol == 24) {
            // steampipe uses short
            write_int("Unknown:", ptr->unknown);
        }
        else if (demo_info.demo_protocol == 4) {
            // protocol 4 uses float
            write_float("Unknown", *(float*)&ptr->unknown);
        }
    }
}
DECL_NET_FREE_FUNC(SvcVoiceInit) {
    DECL_PTR(SvcVoiceInit);
    free(ptr->codec);
}

// SvcVoiceData
DECL_NET_PARSE_FUNC(SvcVoiceData) {
    DECL_PTR(SvcVoiceData);
    ptr->client = bits_read_le_u8(bits);
    ptr->proximity = bits_read_le_u8(bits);
    ptr->length = bits_read_le_u16(bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcVoiceData) {
    const DECL_PTR(SvcVoiceData);
    write_int("Client", ptr->client);
    write_int("Proximity", ptr->proximity);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcVoiceData) {
    DECL_PTR(SvcVoiceData);
    free(ptr->data);
}

// SvcPrint
DECL_NET_PARSE_FUNC(SvcPrint) {
    DECL_PTR(SvcPrint);
    ptr->message = bits_read_str(bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcPrint) {
    const DECL_PTR(SvcPrint);
    write_string("Message", ptr->message);
}
DECL_NET_FREE_FUNC(SvcPrint) {
    DECL_PTR(SvcPrint);
    free(ptr->message);
}

// SvcSounds
DECL_NET_PARSE_FUNC(SvcSounds) {
    DECL_PTR(SvcSounds);
    ptr->reliable_sound = bits_read_one_bit(bits);

    if (ptr->reliable_sound) {
        ptr->size = 1;
        ptr->length = bits_read_le_u8(bits);
    }
    else {
        ptr->size = bits_read_le_u8(bits);
        ptr->length = bits_read_le_u16(bits);
    }

    // SoundInfo[] parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcSounds) {
    const DECL_PTR(SvcSounds);
    write_bool("ReliableSound", ptr->reliable_sound);
    write_int("Size", ptr->size);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcSounds) {
    DECL_PTR(SvcSounds);
    free(ptr->data);
}

// SvcSetView
DECL_NET_PARSE_FUNC(SvcSetView) {
    DECL_PTR(SvcSetView);
    ptr->entity_index = bits_read_bits(11, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcSetView) {
    const DECL_PTR(SvcSetView);
    write_int("EntityIndex", ptr->entity_index);
}
DECL_NET_FREE_FUNC(SvcSetView) {}

// SvcFixAngle
DECL_NET_PARSE_FUNC(SvcFixAngle) {
    DECL_PTR(SvcFixAngle);
    ptr->relative = bits_read_one_bit(bits);
    ptr->angle[0] = bits_read_bit_angle(16, bits);
    ptr->angle[1] = bits_read_bit_angle(16, bits);
    ptr->angle[2] = bits_read_bit_angle(16, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcFixAngle) {
    const DECL_PTR(SvcFixAngle);
    write_bool("Relative", ptr->relative);
    write_vec3("Angles", ptr->angle);
}
DECL_NET_FREE_FUNC(SvcFixAngle) {}

// SvcCrosshairAngle
DECL_NET_PARSE_FUNC(SvcCrosshairAngle) {
    DECL_PTR(SvcCrosshairAngle);
    ptr->angle[0] = bits_read_bit_angle(16, bits);
    ptr->angle[1] = bits_read_bit_angle(16, bits);
    ptr->angle[2] = bits_read_bit_angle(16, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcCrosshairAngle) {
    const DECL_PTR(SvcCrosshairAngle);
    write_vec3("Angles", ptr->angle);
}
DECL_NET_FREE_FUNC(SvcCrosshairAngle) {}

// SvcBspDecal
DECL_NET_PARSE_FUNC(SvcBspDecal) {
    DECL_PTR(SvcBspDecal);
    ptr->pos = bits_read_vcoord(bits);
    ptr->decal_texture_index = bits_read_bits(9, bits);
    ptr->has_entity_index = bits_read_one_bit(bits);
    if (ptr->has_entity_index) {
        ptr->entity_index = bits_read_bits(11, bits);
        ptr->model_index = bits_read_bits(11, bits);
    }
    ptr->low_priority = bits_read_one_bit(bits);
    return true;
}

static float bitcoord_to_f32(BitCoord n) {
    if (!n.exists)
        return 0.0f;
    float value = n.int_value + n.frac_value * (1.0f / (1 << COORD_FRACTIONAL_BITS));
    if (n.sign) {
        value = -value;
    }
    return value;
}

DECL_NET_PRINT_FUNC(SvcBspDecal) {
    const DECL_PTR(SvcBspDecal);
    if (ptr->pos.x.exists)
        write_float("PosX", bitcoord_to_f32(ptr->pos.x));
    if (ptr->pos.y.exists)
        write_float("PosY", bitcoord_to_f32(ptr->pos.y));
    if (ptr->pos.z.exists)
        write_float("PosZ", bitcoord_to_f32(ptr->pos.z));
    if (ptr->has_entity_index) {
        write_int("EntityIndex", ptr->entity_index);
        write_int("ModelIndex", ptr->model_index);
    }
    write_bool("LowPriority", ptr->low_priority);
}
DECL_NET_FREE_FUNC(SvcBspDecal) {}

// SvcSplitScreen
DECL_NET_PARSE_FUNC(SvcSplitScreen) {
    DECL_PTR(SvcSplitScreen);
    ptr->remove_user = bits_read_one_bit(bits);
    ptr->slot = bits_read_bits(11, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcSplitScreen) {
    const DECL_PTR(SvcSplitScreen);
    write_bool("RemoveUser", ptr->remove_user);
    write_int("Slot", ptr->slot);
}
DECL_NET_FREE_FUNC(SvcSplitScreen) {}

// SvcUserMessage
DECL_NET_PARSE_FUNC(SvcUserMessage) {
    DECL_PTR(SvcUserMessage);
    ptr->msg_type = bits_read_le_u8(bits);
    ptr->length = bits_read_bits((demo_info.demo_protocol >= 4) ? 12 : 11, bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcUserMessage) {
    const DECL_PTR(SvcUserMessage);
    write_int("MsgType", ptr->msg_type);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcUserMessage) {
    DECL_PTR(SvcUserMessage);
    free(ptr->data);
}

// SvcEntityMessage
DECL_NET_PARSE_FUNC(SvcEntityMessage) {
    DECL_PTR(SvcEntityMessage);
    ptr->entity_index = bits_read_bits(11, bits);
    ptr->class_id = bits_read_bits(9, bits);
    ptr->length = bits_read_bits(11, bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcEntityMessage) {
    const DECL_PTR(SvcEntityMessage);
    write_int("EntityIndex", ptr->entity_index);
    write_int("ClassId", ptr->class_id);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcEntityMessage) {
    DECL_PTR(SvcEntityMessage);
    free(ptr->data);
}

// SvcGameEvent
DECL_NET_PARSE_FUNC(SvcGameEvent) {
    DECL_PTR(SvcGameEvent);
    ptr->length = bits_read_bits(11, bits);
    // GameEvent[] parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcGameEvent) {
    const DECL_PTR(SvcGameEvent);;
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcGameEvent) {
    DECL_PTR(SvcGameEvent);
    free(ptr->data);
}

// SvcPacketEntities
DECL_NET_PARSE_FUNC(SvcPacketEntities) {
    DECL_PTR(SvcPacketEntities);
    ptr->max_entries = bits_read_bits(11, bits);
    ptr->is_delta = bits_read_one_bit(bits);
    if (ptr->is_delta) {
        ptr->delta_from = bits_read_le_u32(bits);
    }
    ptr->base_line = bits_read_one_bit(bits);
    ptr->updated_entries = bits_read_bits(11, bits);
    ptr->length = bits_read_bits(20, bits);
    ptr->update_baseline = bits_read_one_bit(bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcPacketEntities) {
    const DECL_PTR(SvcPacketEntities);
    write_int("MaxEntries", ptr->max_entries);
    write_bool("IsDelta", ptr->is_delta);
    if (ptr->is_delta) {
        write_int("DeltaFrom", ptr->delta_from);
    }
    write_bool("BaseLine", ptr->base_line);
    write_int("UpdatedEntries", ptr->updated_entries);
    write_int("Length", ptr->length);
    write_bool("UpdateBaseline", ptr->update_baseline);
}
DECL_NET_FREE_FUNC(SvcPacketEntities) {
    DECL_PTR(SvcPacketEntities);
    free(ptr->data);
}

// SvcTempEntities
DECL_NET_PARSE_FUNC(SvcTempEntities) {
    DECL_PTR(SvcTempEntities);
    ptr->num_entries = bits_read_le_u8(bits);

    if (demo_info.network_protocol == 24) {
        ptr->length = bits_read_varuint32(bits);
    }
    else {
        ptr->length = bits_read_bits(17, bits);
    }

    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcTempEntities) {
    const DECL_PTR(SvcTempEntities);
    write_int("NumEntries", ptr->num_entries);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcTempEntities) {
    DECL_PTR(SvcTempEntities);
    free(ptr->data);
}

// SvcPrefetch
DECL_NET_PARSE_FUNC(SvcPrefetch) {
    DECL_PTR(SvcPrefetch);
    ptr->sound_index = bits_read_bits((demo_info.network_protocol == 24) ? 14 : 13, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcPrefetch) {
    const DECL_PTR(SvcPrefetch);
    write_int("SoundIndex", ptr->sound_index);
}
DECL_NET_FREE_FUNC(SvcPrefetch) {}

// SvcMenu
DECL_NET_PARSE_FUNC(SvcMenu) {
    DECL_PTR(SvcMenu);
    ptr->menu_type = bits_read_le_u16(bits);
    ptr->length = bits_read_le_u32(bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcMenu) {
    const DECL_PTR(SvcMenu);
    write_int("MenuType", ptr->menu_type);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcMenu) {
    DECL_PTR(SvcMenu);
    free(ptr->data);
}

// SvcGameEventList
DECL_NET_PARSE_FUNC(SvcGameEventList) {
    DECL_PTR(SvcGameEventList);
    ptr->events = bits_read_bits(9, bits);
    ptr->length = bits_read_bits(20, bits);
    // GameEventDescriptor parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcGameEventList) {
    const DECL_PTR(SvcGameEventList);
    write_int("Event", ptr->events);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcGameEventList) {
    DECL_PTR(SvcGameEventList);
    free(ptr->data);
}

// SvcGetCvarValue
DECL_NET_PARSE_FUNC(SvcGetCvarValue) {
    DECL_PTR(SvcGetCvarValue);
    bits_read_bytes(ptr->cookie, 4, bits);
    ptr->cvar_name = bits_read_str(bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcGetCvarValue) {
    const DECL_PTR(SvcGetCvarValue);
    write_string("Cookie", ptr->cookie);
    write_string("CvarName", ptr->cvar_name);
}
DECL_NET_FREE_FUNC(SvcGetCvarValue) {
    DECL_PTR(SvcGetCvarValue);
    free(ptr->cvar_name);
}

// SvcCmdKeyValues
DECL_NET_PARSE_FUNC(SvcCmdKeyValues) {
    DECL_PTR(SvcCmdKeyValues);
    ptr->length = bits_read_le_u32(bits);
    // data parsing not implemented
    ptr->data = malloc_s(ptr->length);
    bits_read_bytes((char*)ptr->data, ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcCmdKeyValues) {
    const DECL_PTR(SvcPaintmapData);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcCmdKeyValues) {
    DECL_PTR(SvcPaintmapData);
    free(ptr->data);
}

// SvcPaintmapData
DECL_NET_PARSE_FUNC(SvcPaintmapData) {
    DECL_PTR(SvcPaintmapData);
    ptr->length = bits_read_le_u32(bits);
    // data parsing not implemented
    ptr->data = bits_read_bits_arr(ptr->length, bits);
    return true;
}
DECL_NET_PRINT_FUNC(SvcPaintmapData) {
    const DECL_PTR(SvcPaintmapData);
    write_int("Length", ptr->length);
}
DECL_NET_FREE_FUNC(SvcPaintmapData) {
    DECL_PTR(SvcPaintmapData);
    free(ptr->data);
}

// Invalid
DECL_NET_PARSE_FUNC(NetSvcInvalid) {
    return false;
}
DECL_NET_PRINT_FUNC(NetSvcInvalid) {}
DECL_NET_FREE_FUNC(NetSvcInvalid) {}

// settings
const NetSvcMessageSettings oe_net_msg_settings = {
    {MACRO_OE_NET_MESSAGES(DECL_MSG_IN_NAME)},
    {MACRO_OE_NET_MESSAGES(DECL_MSG_IN_ENUM)},
    {MACRO_OE_NET_MESSAGES(DECL_MSG_IN_TABLE)}
};
const NetSvcMessageSettings ne_net_msg_settings = {
    {MACRO_NE_NET_MESSAGES(DECL_MSG_IN_NAME)},
    {MACRO_NE_NET_MESSAGES(DECL_MSG_IN_ENUM)},
    {MACRO_NE_NET_MESSAGES(DECL_MSG_IN_TABLE)}
};