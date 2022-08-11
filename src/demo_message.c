#include "demo_message.h"
#include "demo_info.h"
#include "alloc.h"
#include "print.h"

#define DECL_PARSE_FUNC(type) static bool PARSE_FUNC_NAME(type)(DemoMessageData* thisptr, BitStream* bits)
#define DECL_PRINT_FUNC(type) static void PRINT_FUNC_NAME(type)(const DemoMessageData* thisptr, FILE* fp)
#define DECL_FREE_FUNC(type) static void FREE_FUNC_NAME(type)(DemoMessageData* thisptr)

// SignOn

static void parse_cmd_info(CmdInfo* info, BitStream* bits) {
    info->flags = bits_read_le_u32(bits);
    info->view_origin[0] = bits_read_le_f32(bits);
    info->view_origin[1] = bits_read_le_f32(bits);
    info->view_origin[2] = bits_read_le_f32(bits);
    info->view_angles[0] = bits_read_le_f32(bits);
    info->view_angles[1] = bits_read_le_f32(bits);
    info->view_angles[2] = bits_read_le_f32(bits);
    info->local_view_angles[0] = bits_read_le_f32(bits);
    info->local_view_angles[1] = bits_read_le_f32(bits);
    info->local_view_angles[2] = bits_read_le_f32(bits);
    info->view_origin2[0] = bits_read_le_f32(bits);
    info->view_origin2[1] = bits_read_le_f32(bits);
    info->view_origin2[2] = bits_read_le_f32(bits);
    info->view_angles2[0] = bits_read_le_f32(bits);
    info->view_angles2[1] = bits_read_le_f32(bits);
    info->view_angles2[2] = bits_read_le_f32(bits);
    info->local_view_angles2[0] = bits_read_le_f32(bits);
    info->local_view_angles2[1] = bits_read_le_f32(bits);
    info->local_view_angles2[2] = bits_read_le_f32(bits);
}

DECL_PARSE_FUNC(SignOn) {
    DECL_PTR(SignOn);
    if (!demo_info.quick_mode) {
        for (int i = 0; i < demo_info.MSSC; i++) {
            parse_cmd_info(&ptr->packet_info[i], bits);
        }
        ptr->in_sequence = bits_read_le_u32(bits);
        ptr->out_sequence = bits_read_le_u32(bits);
    }
    else {
        // PacketInfo(76) * MSSC + InSequence(4) + OutSequence(4)
        bits_skip((76 * demo_info.MSSC + 8) << 3, bits);
    }
    size_t byte_size = ptr->size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        bool success = true;
        while (end_index - bits->current > 6) {
            NetSvcMessage msg = { 0 };
            // parse net message
            NetSvcMessageType type = msg.type = bits_read_bits(6, bits);
            if (type < NET_MSG_COUNT && demo_info.net_msg_ids[type] != NetInvalid_MSG) {
                debug("Parsing net message type %d.\n", type);
                success = demo_info.net_msg_table[type].parse(&msg.data, bits);
                if (!success) {
                    warning("Failed to parse NET/SVC message tpye %d at %d.\n", type, (uint32_t)ptr->data.size + 1);
                    break;
                }
                vector_push(ptr->data, msg);
            }
            else {
                success = false;
                warning("Unexpected NET/SVC message type %d at %d.\n", type, (uint32_t)ptr->data.size + 1);
                break;
            }
        }
        vector_shrink(ptr->data);
        if (bits->current > end_index || !success) {
            warning("NET/SVC Message not parse correctly.\n");
        }
    }
    bits_setpos(end_index, bits);
    return true;
}

static void print_cmd_info(const CmdInfo* info, FILE* fp) {
    fprintf(fp, "\t\tFlags: %d\n", info->flags);
    fprintf(fp, "\t\tViewOrigin: (%.3f, %.3f, %.3f)\n", info->view_origin[0], info->view_origin[1], info->view_origin[2]);
    fprintf(fp, "\t\tViewAngles: (%.3f, %.3f, %.3f)\n", info->view_angles[0], info->view_angles[1], info->view_angles[2]);
    fprintf(fp, "\t\tLocalViewAngles: (%.3f, %.3f, %.3f)\n", info->local_view_angles[0], info->local_view_angles[1], info->local_view_angles[2]);
    fprintf(fp, "\t\tViewOrigin2: (%.3f, %.3f, %.3f)\n", info->view_origin2[0], info->view_origin2[1], info->view_origin2[2]);
    fprintf(fp, "\t\tViewAngles2: (%.3f, %.3f, %.3f)\n", info->view_angles2[0], info->view_angles2[1], info->view_angles2[2]);
    fprintf(fp, "\t\tLocalViewAngles2: (%.3f, %.3f, %.3f)\n", info->local_view_angles2[0], info->local_view_angles2[1], info->local_view_angles2[2]);
}

static void print_packet_data(const DemoMessageData* thisptr, FILE* fp) {
    const DECL_PTR(Packet);
    for (int i = 0; i < demo_info.MSSC; i++) {
        fprintf(fp, "\tPacketInfo[%d]:\n", i);
        print_cmd_info(&ptr->packet_info[i], fp);
    }
    fprintf(fp, "\tInSequence: %d\n", ptr->in_sequence);
    fprintf(fp, "\tOutSequence: %d\n", ptr->out_sequence);
    for (size_t i = 0; i < ptr->data.size; i++) {
        NetSvcMessage* msg = &ptr->data.data[i];
        fprintf(fp, "\tNET/SVC-Message[%d]:\n", (uint32_t)i);
        demo_info.net_msg_table[msg->type].print(&msg->data, fp);
    }
}

DECL_PRINT_FUNC(SignOn) {
    fprintf(fp, "SignOn\n");
    print_packet_data(thisptr, fp);
}
DECL_FREE_FUNC(SignOn) {
    DECL_PTR(SignOn);
    if (!demo_info.quick_mode) {
        for (size_t i = 0; i < ptr->data.size; i++) {
            NetSvcMessage* msg = &ptr->data.data[i];
            demo_info.net_msg_table[msg->type].free(&msg->data);
        }
    }
}

// Packet
DECL_PARSE_FUNC(Packet) {
    return parse_SignOn(thisptr, bits);
}
DECL_PRINT_FUNC(Packet) {
    fprintf(fp, "Packet\n");
    print_packet_data(thisptr, fp);
}
DECL_FREE_FUNC(Packet) {
    free_SignOn(thisptr);
}

// SyncTick
DECL_PARSE_FUNC(SyncTick) {
    return true;
}
DECL_PRINT_FUNC(SyncTick) {
    fprintf(fp, "SyncTick\n");
}
DECL_FREE_FUNC(SyncTick) {}

// ConsoleCmd
DECL_PARSE_FUNC(ConsoleCmd) {
    DECL_PTR(ConsoleCmd);
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        ptr->size = byte_size;
        ptr->data = (char*)malloc_s(byte_size);
        bits_read_bytes(ptr->data, byte_size, bits);
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(ConsoleCmd) {
    const DECL_PTR(ConsoleCmd);
    fprintf(fp, "ConsoleCmd\n");
    fprintf(fp, "\tData: %s\n", (char*)ptr->data);
}
DECL_FREE_FUNC(ConsoleCmd) {
    DECL_PTR(ConsoleCmd);
    if (!demo_info.quick_mode) {
        free(ptr->data);
    }
}

// UserCmd

static void parse_usercmd(UserCmdInfo* cmd, BitStream* bits) {
    if ((cmd->has_command_number = bits_read_one_bit(bits)))
        cmd->command_number = bits_read_le_u32(bits);

    if ((cmd->has_tick_count = bits_read_one_bit(bits)))
        cmd->tick_count = bits_read_le_u32(bits);

    if ((cmd->has_view_angles_x = bits_read_one_bit(bits)))
        cmd->view_angles_x = bits_read_le_f32(bits);
    if ((cmd->has_view_angles_y = bits_read_one_bit(bits)))
        cmd->view_angles_y = bits_read_le_f32(bits);
    if ((cmd->has_view_angles_z = bits_read_one_bit(bits)))
        cmd->view_angles_z = bits_read_le_f32(bits);

    if ((cmd->has_forward_move = bits_read_one_bit(bits)))
        cmd->forward_move = bits_read_le_f32(bits);
    if ((cmd->has_side_move = bits_read_one_bit(bits)))
        cmd->side_move = bits_read_le_f32(bits);
    if ((cmd->has_up_move = bits_read_one_bit(bits)))
        cmd->up_move = bits_read_le_f32(bits);

    if ((cmd->has_buttons = bits_read_one_bit(bits)))
        cmd->buttons = bits_read_le_u32(bits);

    if ((cmd->has_impulse = bits_read_one_bit(bits)))
        cmd->impulse = bits_read_le_u8(bits);

    if ((cmd->has_weapon_select = bits_read_one_bit(bits))) {
        cmd->weapon_select = bits_read_bits(11, bits);
        if ((cmd->has_weapon_subtype = bits_read_one_bit(bits)))
            cmd->weapon_subtype = bits_read_bits(6, bits);
    }

    if ((cmd->has_mouse_dx = bits_read_one_bit(bits)))
        cmd->mouse_dx = bits_read_le_u16(bits);
    if ((cmd->has_mouse_dy = bits_read_one_bit(bits)))
        cmd->mouse_dy = bits_read_le_u16(bits);
}

DECL_PARSE_FUNC(UserCmd) {
    DECL_PTR(UserCmd);
    uint32_t cmd = bits_read_le_u32(bits);
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        ptr->cmd = cmd;
        ptr->size = byte_size;
        parse_usercmd(&ptr->data, bits);
        if (bits->current > end_index) {
            warning("Usercmd not parse correctly.\n");
        }
    }
    bits_setpos(end_index, bits);
    return true;
}

DECL_PRINT_FUNC(UserCmd) {
    const DECL_PTR(UserCmd);
    fprintf(fp, "UserCmd\n");
    const UserCmdInfo* cmd = &ptr->data;
    if (cmd->has_command_number)
        fprintf(fp, "\tCommandNumber: %d\n", cmd->command_number);
    if (cmd->has_tick_count)
        fprintf(fp, "\tTickCount: %d\n", cmd->tick_count);
    if (cmd->has_view_angles_x)
        fprintf(fp, "\tViewAnglesX: %.3f\n", cmd->view_angles_x);
    if (cmd->has_view_angles_y)
        fprintf(fp, "\tViewAnglesY: %.3f\n", cmd->view_angles_y);
    if (cmd->has_view_angles_z)
        fprintf(fp, "\tViewAnglesZ: %.3f\n", cmd->view_angles_z);
    if (cmd->has_forward_move)
        fprintf(fp, "\tForwardMove: %.3f\n", cmd->forward_move);
    if (cmd->has_side_move)
        fprintf(fp, "\tSideMove: %.3f\n", cmd->side_move);
    if (cmd->has_up_move)
        fprintf(fp, "\tUpMove: %.3f\n", cmd->up_move);
    if (cmd->has_buttons)
        fprintf(fp, "\tButtons: %d\n", cmd->buttons);
    if (cmd->has_impulse)
        fprintf(fp, "\tImpulse: %d\n", cmd->impulse);
    if (cmd->has_weapon_select) {
        fprintf(fp, "\tWeaponSelect: %d\n", cmd->weapon_select);
        if (cmd->has_weapon_subtype)
            fprintf(fp, "\tWeaponSubtype: %d\n", cmd->weapon_subtype);
    }
    if (cmd->has_mouse_dx)
        fprintf(fp, "\tMouseDx: %d\n", cmd->mouse_dx);
    if (cmd->has_mouse_dy)
        fprintf(fp, "\tMouseDy: %d\n", cmd->mouse_dy);
}
DECL_FREE_FUNC(UserCmd) {}

// DataTables
DECL_PARSE_FUNC(DataTables) {
    DECL_PTR(DataTables);
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        // not implement
        ptr->size = byte_size;
        bits_skip(byte_size << 3, bits);
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(DataTables) {
    fprintf(fp, "DataTables\n");
}
DECL_FREE_FUNC(DataTables) {}

// Stop
DECL_PARSE_FUNC(Stop) {
    return true;
}
DECL_PRINT_FUNC(Stop) {
    fprintf(fp, "Stop\n");
}
DECL_FREE_FUNC(Stop) {}

// CustomData
DECL_PARSE_FUNC(CustomData) {
    DECL_PTR(CustomData);
    ptr->unknown = bits_read_le_u32(bits);
    uint32_t byte_size = ptr->size = bits_read_le_u32(bits);
    ptr->data = (uint8_t*)malloc_s(byte_size);
    bits_read_bytes((char*)ptr->data, byte_size, bits);
    return true;
}
DECL_PRINT_FUNC(CustomData) {
    const DECL_PTR(CustomData);
    fprintf(fp, "CustomData\n");
    fprintf(fp, "\tUnknown: %d\n", ptr->unknown);
    fprintf(fp, "\tSize: %d\n", ptr->size);
}
DECL_FREE_FUNC(CustomData) {
    DECL_PTR(CustomData);
    free(ptr->data);
}

// StringTables
DECL_PARSE_FUNC(StringTables) {
    DECL_PTR(StringTables);
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        // not implement
        ptr->size = byte_size;
        bits_skip(byte_size << 3, bits);
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(StringTables) {
    fprintf(fp, "StringTables\n");
}
DECL_FREE_FUNC(StringTables) {}

// Invalid
DECL_PARSE_FUNC(Invalid) {
    return false;
}
DECL_PRINT_FUNC(Invalid) {}
DECL_FREE_FUNC(Invalid) {}

// // function tables
const DemoMessageTable portal_3420_massage_table[MESSAGE_COUNT] = {
    MACRO_PORTAL_3420_MESSAGES(DECL_MSG_IN_TABLE)
};
const DemoMessageTable portal_5135_massage_table[MESSAGE_COUNT] = {
    MACRO_PORTAL_5135_MESSAGES(DECL_MSG_IN_TABLE)
};
const DemoMessageTable ne_massage_table[MESSAGE_COUNT] = {
    MACRO_NE_MESSAGES(DECL_MSG_IN_TABLE)
};

const DemoMessageID portal_3420_massage_ids[MESSAGE_COUNT] = {
    MACRO_PORTAL_3420_MESSAGES(DECL_MSG_IN_ENUM)
};
const DemoMessageID portal_5135_massage_ids[MESSAGE_COUNT] = {
    MACRO_PORTAL_5135_MESSAGES(DECL_MSG_IN_ENUM)
};
const DemoMessageID ne_massage_ids[MESSAGE_COUNT] = {
    MACRO_NE_MESSAGES(DECL_MSG_IN_ENUM)
};