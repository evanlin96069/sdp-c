#include "demo_message.h"
#include "demo_info.h"
#include "alloc.h"

#define DECL_PARSE_FUNC(type) static void PARSE_FUNC_NAME(type)(DemoMessage* msg, BitStream* bits)
#define DECL_PRINT_FUNC(type) static void  PRINT_FUNC_NAME(type)(const DemoMessage* msg, FILE* fp)
#define DECL_FREE_FUNC(type) static void  FREE_FUNC_NAME(type)(DemoMessage* msg)

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
    if (!demo_info.quick_mode) {
        for (int i = 0; i < demo_info.MSSC; i++) {
            parse_cmd_info(&msg->data.Packet_message.packet_info[i], bits);
        }
        msg->data.Packet_message.in_sequence = bits_read_le_u32(bits);
        msg->data.Packet_message.out_sequence = bits_read_le_u32(bits);
    }
    else {
        // PacketInfo(76) * MSSC + InSequence(4) + OutSequence(4)
        bits_skip((76 * demo_info.MSSC + 8) << 3, bits);
    }
    size_t byte_size = msg->data.Packet_message.size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        while (end_index - bits->current > 6) {
            NetSvcMessage net_message;
            net_message.type = bits_read_bits(6, bits);
            demo_info.net_msg_table[net_message.type].parse(&net_message, bits);
            vector_push(msg->data.SignOn_message.data, net_message);
        }
    }
    if (bits->current > end_index) {
        fprintf(stderr, "[WARNING] NET/SVC Message not parse correctly.\n");
    }
    bits->current = end_index;
    bits_fetch(bits);
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

static void print_packet_data(const DemoMessage* msg, FILE* fp) {
    for (int i = 0; i < demo_info.MSSC; i++) {
        fprintf(fp, "\tPacketInfo[%d]:\n", i);
        print_cmd_info(&msg->data.Packet_message.packet_info[i], fp);
    }
    fprintf(fp, "\tInSequence: %d\n", msg->data.Packet_message.in_sequence);
    fprintf(fp, "\tOutSequence: %d\n", msg->data.Packet_message.out_sequence);
    const Vector_NetSvcMessage* net_message = &msg->data.Packet_message.data;
    for (size_t i = 0; i < net_message->size; i++) {
        fprintf(fp, "\tNET/SVC-Message[%d]:\n", (uint32_t)i);
        demo_info.net_msg_table[net_message->data[i].type].print(&net_message->data[i], fp);
    }
}

DECL_PRINT_FUNC(SignOn) {
    fprintf(fp, "SignOn\n");
    print_packet_data(msg, fp);
}
DECL_FREE_FUNC(SignOn) {
    if (!demo_info.quick_mode) {
        Vector_NetSvcMessage* net_message = &msg->data.Packet_message.data;
        for (size_t i = 0; i < net_message->size; i++) {
            demo_info.net_msg_table[net_message->data[i].type].free(&net_message->data[i]);
        }
    }
}

// Packet
DECL_PARSE_FUNC(Packet) {
    parse_SignOn(msg, bits);
}
DECL_PRINT_FUNC(Packet) {
    fprintf(fp, "Packet\n");
    print_packet_data(msg, fp);
}
DECL_FREE_FUNC(Packet) {
    free_SignOn(msg);
}

// SyncTick
DECL_PARSE_FUNC(SyncTick) {}
DECL_PRINT_FUNC(SyncTick) {
    fprintf(fp, "SyncTick\n");
}
DECL_FREE_FUNC(SyncTick) {}

// ConsoleCmd
DECL_PARSE_FUNC(ConsoleCmd) {
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        msg->data.ConsoleCmd_message.size = byte_size;
        msg->data.ConsoleCmd_message.data = (char*)malloc_s(byte_size);
        bits_read_bytes(msg->data.ConsoleCmd_message.data, byte_size, bits);
    }
    bits->current = end_index;
    bits_fetch(bits);
}
DECL_PRINT_FUNC(ConsoleCmd) {
    fprintf(fp, "ConsoleCmd\n");
    fprintf(fp, "\tData: %s\n", (char*)msg->data.ConsoleCmd_message.data);
}
DECL_FREE_FUNC(ConsoleCmd) {
    if (!demo_info.quick_mode) {
        free(msg->data.ConsoleCmd_message.data);
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
    uint32_t cmd = bits_read_le_u32(bits);
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        msg->data.UserCmd_message.cmd = cmd;
        msg->data.UserCmd_message.size = byte_size;
        parse_usercmd(&msg->data.UserCmd_message.data, bits);
        if (bits->current > end_index) {
            fprintf(stderr, "[WARNING] Usercmd not parse correctly.\n");
        }
    }
    bits->current = end_index;
    bits_fetch(bits);
}

DECL_PRINT_FUNC(UserCmd) {
    fprintf(fp, "UserCmd\n");
    const UserCmdInfo* cmd = &msg->data.UserCmd_message.data;
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

DECL_PARSE_FUNC(DataTables) {
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        // not implement
        msg->data.DataTables_message.size = byte_size;
        bits_skip(byte_size << 3, bits);
    }
    bits->current = end_index;
    bits_fetch(bits);
}
DECL_PRINT_FUNC(DataTables) {
    fprintf(fp, "DataTables\n");
}
DECL_FREE_FUNC(DataTables) {}

DECL_PARSE_FUNC(Stop) {}
DECL_PRINT_FUNC(Stop) {
    fprintf(fp, "Stop\n");
}
DECL_FREE_FUNC(Stop) {}

DECL_PARSE_FUNC(CustomData) {
    msg->data.CustomData_message.unknown = bits_read_le_u32(bits);
    uint32_t byte_size = msg->data.CustomData_message.size = bits_read_le_u32(bits);
    msg->data.CustomData_message.data = (uint8_t*)malloc_s(byte_size);
    bits_read_bytes((char*)msg->data.CustomData_message.data, byte_size, bits);
}
DECL_PRINT_FUNC(CustomData) {
    fprintf(fp, "CustomData\n");
    fprintf(fp, "\tUnknown: %d\n", msg->data.CustomData_message.unknown);
    fprintf(fp, "\tSize: %d\n", msg->data.CustomData_message.size);
}
DECL_FREE_FUNC(CustomData) {
    free(msg->data.CustomData_message.data);
}

DECL_PARSE_FUNC(StringTables) {
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (!demo_info.quick_mode) {
        // not implement
        msg->data.StringTables_message.size = byte_size;
        bits_skip(byte_size << 3, bits);
    }
    bits->current = end_index;
    bits_fetch(bits);
}
DECL_PRINT_FUNC(StringTables) {
    fprintf(fp, "StringTables\n");
}
DECL_FREE_FUNC(StringTables) {}

DECL_PARSE_FUNC(Invalid) {}
DECL_PRINT_FUNC(Invalid) {}
DECL_FREE_FUNC(Invalid) {}

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