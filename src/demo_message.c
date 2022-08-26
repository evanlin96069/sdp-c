#include "demo_message.h"
#include "demo_info.h"
#include "alloc.h"
#include "print.h"

#define DECL_PARSE_FUNC(type) static bool PARSE_FUNC_NAME(type)(DemoMessageData* thisptr, BitStream* bits)
#define DECL_PRINT_FUNC(type) static void PRINT_FUNC_NAME(type)(const DemoMessageData* thisptr, FILE* fp)
#define DECL_FREE_FUNC(type) static void FREE_FUNC_NAME(type)(DemoMessageData* thisptr)

// Packet

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

DECL_PARSE_FUNC(Packet) {
    DECL_PTR(Packet);
    for (int i = 0; i < demo_info.MSSC; i++) {
        parse_cmd_info(&ptr->packet_info[i], bits);
    }
    ptr->in_sequence = bits_read_le_u32(bits);
    ptr->out_sequence = bits_read_le_u32(bits);

    size_t byte_size = ptr->size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (demo_info.parse_level >= 2) {
        bool success = true;
        uint32_t count = 0;
        ptr->data.capacity = 0;
        ptr->data.size = 0;
        while (end_index - bits->current > 6) {
            count++;
            NetSvcMessage msg = { 0 };
            // parse net message
            NetSvcMessageType type = msg.type = bits_read_bits((demo_info.network_protocol <= 14) ? 5 : 6, bits);
            if (type < NET_MSG_COUNT) {
                const char* name = demo_info.net_msg_settings->names[type];
                debug("Parsing NET/SVC message %s (%d) at %d.\n", name, type, count);
                success = demo_info.net_msg_settings->func_table[type].parse(&msg.data, bits);
                if (!success) {
                    warning("Failed to parse NET/SVC message %s (%d) at %d.\n", name, type, count);
                    break;
                }
                vector_push(ptr->data, msg);
            }
            else {
                success = false;
                warning("Unexpected NET/SVC message type %d at %d.\n", type, count);
                break;
            }
        }
        vector_shrink(ptr->data);
        if (bits->current > end_index || !success) {
            warning("Packet not parsed correctly.\n");
        }
        else if (end_index - bits->current >= 8) {
            warning("Had more than 1 byte remain after parsing Packet.\n");
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

DECL_PRINT_FUNC(Packet) {
    const DECL_PTR(Packet);
    for (int i = 0; i < demo_info.MSSC; i++) {
        fprintf(fp, "\tPacketInfo[%d]:\n", i);
        print_cmd_info(&ptr->packet_info[i], fp);
    }
    fprintf(fp, "\tInSequence: %d\n", ptr->in_sequence);
    fprintf(fp, "\tOutSequence: %d\n", ptr->out_sequence);

    if (demo_info.parse_level >= 2) {
        fprintf(fp, "\tNET/SVC-Messages:\n");
        for (size_t i = 0; i < ptr->data.size; i++) {
            NetSvcMessage* msg = &ptr->data.data[i];
            NetSvcMessageType type = msg->type;
            const char* name = demo_info.net_msg_settings->names[type];
            fprintf(fp, "\t\t%s (%d)\n", name, type);
            demo_info.net_msg_settings->func_table[type].print(&msg->data, fp);
        }
    }
}
DECL_FREE_FUNC(Packet) {
    DECL_PTR(Packet);
    if (demo_info.parse_level >= 2) {
        for (size_t i = 0; i < ptr->data.size; i++) {
            NetSvcMessage* msg = &ptr->data.data[i];
            demo_info.net_msg_settings->func_table[msg->type].free(&msg->data);
        }
    }
}

// SignOn
DECL_PARSE_FUNC(SignOn) {
    return parse_Packet(thisptr, bits);
}
DECL_PRINT_FUNC(SignOn) {
    print_Packet(thisptr, fp);
}
DECL_FREE_FUNC(SignOn) {
    free_Packet(thisptr);
}

// SyncTick
DECL_PARSE_FUNC(SyncTick) {
    return true;
}
DECL_PRINT_FUNC(SyncTick) {}
DECL_FREE_FUNC(SyncTick) {}

// ConsoleCmd
DECL_PARSE_FUNC(ConsoleCmd) {
    DECL_PTR(ConsoleCmd);
    ptr->size = bits_read_le_u32(bits);
    ptr->data = (char*)malloc_s(ptr->size);
    bits_read_bytes(ptr->data, ptr->size, bits);
    return true;
}
DECL_PRINT_FUNC(ConsoleCmd) {
    const DECL_PTR(ConsoleCmd);
    fprintf(fp, "\tData: %s\n", (char*)ptr->data);
}
DECL_FREE_FUNC(ConsoleCmd) {
    DECL_PTR(ConsoleCmd);
    free(ptr->data);
}

// UserCmd

static void parse_usercmd(UserCmdInfo* cmd, BitStream* bits) {
    if ((cmd->has_command_number = bits_read_one_bit(bits)))
        cmd->command_number = bits_read_le_u32(bits);
    if (demo_info.network_protocol <= 7) {
        cmd->has_tick_count = bits_read_one_bit(bits);
        if (cmd->has_tick_count)
            cmd->tick_count = bits_read_le_u32(bits);
        if ((cmd->has_view_angles_x = bits_read_one_bit(bits)))
            cmd->view_angles_x = bits_read_bit_angle(16, bits);
        if ((cmd->has_view_angles_y = bits_read_one_bit(bits)))
            cmd->view_angles_y = bits_read_bit_angle(16, bits);
        if ((cmd->has_view_angles_z = bits_read_one_bit(bits)))
            cmd->view_angles_z = bits_read_bit_angle(8, bits);

        if ((cmd->has_forward_move = bits_read_one_bit(bits)))
            cmd->forward_move = (int16_t)bits_read_le_u16(bits);
        if ((cmd->has_side_move = bits_read_one_bit(bits)))
            cmd->side_move = (int16_t)bits_read_le_u16(bits);
        if ((cmd->has_up_move = bits_read_one_bit(bits)))
            cmd->up_move = (int16_t)bits_read_le_u16(bits);
    }
    else {
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
    }

    if ((cmd->has_buttons = bits_read_one_bit(bits)))
        cmd->buttons = bits_read_le_u32(bits);

    if ((cmd->has_impulse = bits_read_one_bit(bits)))
        cmd->impulse = bits_read_le_u8(bits);

    if ((cmd->has_weapon_select = bits_read_one_bit(bits))) {
        cmd->weapon_select = bits_read_bits(11, bits);
        if ((cmd->has_weapon_subtype = bits_read_one_bit(bits)))
            cmd->weapon_subtype = bits_read_bits(6, bits);
        if (demo_info.game == DMOMM)
            cmd->mm_unknown_weapon_flags = bits_read_bits(2, bits);
    }

    if (demo_info.game == DMOMM) {
        if ((cmd->has_unknown_b_1 = bits_read_one_bit(bits))) {
            cmd->unknown_b_1 = bits_read_one_bit(bits);
            cmd->unknown_b_2 = bits_read_one_bit(bits);
        }
        if ((cmd->has_unknown_u11 = bits_read_one_bit(bits)))
            cmd->unknown_u11 = bits_read_bits(11, bits);
        if ((cmd->has_mm_move_item_from_slot = bits_read_one_bit(bits)))
            cmd->mm_move_item_from_slot = bits_read_le_u32(bits);
        if ((cmd->has_mm_move_item_to_slot = bits_read_one_bit(bits)))
            cmd->mm_move_item_to_slot = bits_read_le_u32(bits);
        if ((cmd->has_mm_stealth = bits_read_one_bit(bits)))
            cmd->mm_stealth = bits_read_le_f32(bits);
        if ((cmd->has_mm_use_item_id = bits_read_one_bit(bits))) {
            cmd->mm_use_item_id = bits_read_le_u8(bits);
            cmd->mm_unknown_item_flag = bits_read_one_bit(bits);
        }
        if ((cmd->has_unknown_i6 = bits_read_one_bit(bits)))
            cmd->unknown_i6 = bits_read_bits(6, bits);
        if ((cmd->has_mm_upgrade_skill_type = bits_read_one_bit(bits)))
            cmd->mm_upgrade_skill_type = bits_read_le_u8(bits);

        if ((cmd->has_mouse_dx = bits_read_one_bit(bits)))
            cmd->mouse_dx = bits_read_le_u16(bits);
        if ((cmd->has_mouse_dy = bits_read_one_bit(bits)))
            cmd->mouse_dy = bits_read_le_u16(bits);

        // ???
        if ((cmd->has_unknown_i16 = bits_read_one_bit(bits)))
            cmd->unknown_i16 = bits_read_le_u16(bits);

        if ((cmd->has_mm_lean_move = bits_read_one_bit(bits)))
            cmd->mm_lean_move = bits_read_le_f32(bits);

        if ((cmd->has_mm_sprint = bits_read_one_bit(bits)))
            cmd->mm_sprint = bits_read_one_bit(bits);
        if ((cmd->has_mm_unknown_action_2 = bits_read_one_bit(bits)))
            cmd->mm_unknown_action_2 = bits_read_one_bit(bits);
        if ((cmd->has_mm_kick = bits_read_one_bit(bits)))
            cmd->mm_kick = bits_read_one_bit(bits);
        if ((cmd->has_mm_unknown_action_4 = bits_read_one_bit(bits)))
            cmd->mm_unknown_action_4 = bits_read_one_bit(bits);
        if ((cmd->has_mm_unknown_action_5 = bits_read_one_bit(bits)))
            cmd->mm_unknown_action_5 = bits_read_one_bit(bits);
        if ((cmd->has_mm_shwo_charsheet = bits_read_one_bit(bits)))
            cmd->mm_shwo_charsheet = bits_read_one_bit(bits);
        if ((cmd->has_mm_unknown_action_7 = bits_read_one_bit(bits)))
            cmd->mm_unknown_action_7 = bits_read_one_bit(bits);
        if ((cmd->has_mm_show_inventory_belt = bits_read_one_bit(bits)))
            cmd->mm_show_inventory_belt = bits_read_one_bit(bits);
        if ((cmd->has_mm_show_inventory_belt_select = bits_read_one_bit(bits)))
            cmd->mm_show_inventory_belt_select = bits_read_one_bit(bits);
        if ((cmd->has_mm_hide_inventory_belt_select = bits_read_one_bit(bits)))
            cmd->mm_hide_inventory_belt_select = bits_read_one_bit(bits);
        if ((cmd->has_mm_show_objectives = bits_read_one_bit(bits)))
            cmd->mm_show_objectives = bits_read_one_bit(bits);
        if ((cmd->has_mm_hide_objectives = bits_read_one_bit(bits)))
            cmd->mm_hide_objectives = bits_read_one_bit(bits);

        if ((cmd->has_mm_exit_book_id = bits_read_one_bit(bits)))
            cmd->mm_exit_book_id = bits_read_le_u32(bits);
        if ((cmd->has_mm_xana = bits_read_one_bit(bits)))
            cmd->mm_xana = bits_read_one_bit(bits);
    }
    else {
        if ((cmd->has_mouse_dx = bits_read_one_bit(bits)))
            cmd->mouse_dx = bits_read_le_u16(bits);
        if ((cmd->has_mouse_dy = bits_read_one_bit(bits)))
            cmd->mouse_dy = bits_read_le_u16(bits);
    }
}

DECL_PARSE_FUNC(UserCmd) {
    DECL_PTR(UserCmd);
    uint32_t cmd = bits_read_le_u32(bits);
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);

    ptr->cmd = cmd;
    ptr->size = byte_size;
    parse_usercmd(&ptr->data, bits);
    if (bits->current > end_index) {
        warning("Usercmd not parsed correctly.\n");
    }

    /*
    if (end_index - bits->current >= 8) {
        warning("Had more than 1 byte remain after parsing Usercmd.\n");
    }
    */

    bits_setpos(end_index, bits);
    return true;
}

DECL_PRINT_FUNC(UserCmd) {
    const DECL_PTR(UserCmd);
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
        if (demo_info.game == DMOMM)
            fprintf(fp, "\tUnknownWeaponFlags: %d\n", cmd->mm_unknown_weapon_flags);
    }

    if (demo_info.game == DMOMM) {
        if (cmd->has_unknown_b_1) {
            fprintf(fp, "\tUnknown_b_1: %s\n", cmd->unknown_b_1 ? "true" : "false");
            fprintf(fp, "\tUnknown_b_2: %s\n", cmd->unknown_b_2 ? "true" : "false");
        }
        if (cmd->has_unknown_u11)
            fprintf(fp, "\tUnknown_u11: %d\n", cmd->unknown_u11);
        if (cmd->has_mm_move_item_from_slot)
            fprintf(fp, "\tMoveItemFromSlot: %d\n", cmd->mm_move_item_from_slot);
        if (cmd->has_mm_move_item_to_slot)
            fprintf(fp, "\ttMoveItemToSlot: %d\n", cmd->mm_move_item_to_slot);
        if (cmd->has_mm_stealth)
            fprintf(fp, "\tStealth: %.3f\n", cmd->mm_stealth);
        if (cmd->has_mm_use_item_id) {
            fprintf(fp, "\tUseItemId: %d\n", cmd->mm_use_item_id);
            fprintf(fp, "\tUnknownItemFlag: %s\n", cmd->mm_unknown_item_flag ? "true" : "false");
        }
        if (cmd->has_unknown_i6)
            fprintf(fp, "\tUnknown_i6: %d\n", cmd->unknown_i6);
        if (cmd->has_mm_upgrade_skill_type)
            fprintf(fp, "\tUpgradeSkillType: %d\n", cmd->mm_upgrade_skill_type);

        if (cmd->has_mouse_dx)
            fprintf(fp, "\tMouseDx: %d\n", cmd->mouse_dx);
        if (cmd->has_mouse_dy)
            fprintf(fp, "\tMouseDy: %d\n", cmd->mouse_dy);

        // ???
        if (cmd->has_unknown_i16)
            fprintf(fp, "\tUnknown_i16: %d\n", cmd->unknown_i16);

        if (cmd->has_mm_lean_move)
            fprintf(fp, "\tLeanMove: %.3f\n", cmd->mm_lean_move);

        if (cmd->has_mm_sprint)
            fprintf(fp, "\tSprint: %s\n", cmd->mm_sprint ? "true" : "false");
        if (cmd->has_mm_unknown_action_2)
            fprintf(fp, "\tUnknownAction_2: %s\n", cmd->mm_unknown_action_2 ? "true" : "false");
        if (cmd->has_mm_kick)
            fprintf(fp, "\tKick: %s\n", cmd->mm_kick ? "true" : "false");
        if (cmd->has_mm_unknown_action_4)
            fprintf(fp, "\tUnknownAction_4: %s\n", cmd->mm_unknown_action_4 ? "true" : "false");
        if (cmd->has_mm_unknown_action_5)
            fprintf(fp, "\tUnknownAction_5: %s\n", cmd->mm_unknown_action_5 ? "true" : "false");
        if (cmd->has_mm_shwo_charsheet)
            fprintf(fp, "\tShowCharsheet: %s\n", cmd->mm_shwo_charsheet ? "true" : "false");
        if (cmd->has_mm_unknown_action_7)
            fprintf(fp, "\tUnknownAction_7: %s\n", cmd->mm_unknown_action_7 ? "true" : "false");
        if (cmd->has_mm_show_inventory_belt)
            fprintf(fp, "\tShowInventoryBelt: %s\n", cmd->mm_show_inventory_belt ? "true" : "false");
        if (cmd->has_mm_show_inventory_belt_select)
            fprintf(fp, "\tShowInventoryBeltSelect: %s\n", cmd->mm_show_inventory_belt_select ? "true" : "false");
        if (cmd->has_mm_hide_inventory_belt_select)
            fprintf(fp, "\tHideInventoryBeltSelect: %s\n", cmd->mm_hide_inventory_belt_select ? "true" : "false");
        if (cmd->has_mm_show_objectives)
            fprintf(fp, "\tShowObjectives: %s\n", cmd->mm_show_objectives ? "true" : "false");
        if (cmd->has_mm_hide_objectives)
            fprintf(fp, "\tHideObjectives: %s\n", cmd->mm_hide_objectives ? "true" : "false");

        if (cmd->has_mm_exit_book_id)
            fprintf(fp, "\tExitBookId: %d\n", cmd->mm_exit_book_id);
        if (cmd->has_mm_xana)
            fprintf(fp, "\tXana: %s\n", cmd->mm_xana ? "true" : "false");
    }
    else {
        if (cmd->has_mouse_dx)
            fprintf(fp, "\tMouseDx: %d\n", cmd->mouse_dx);
        if (cmd->has_mouse_dy)
            fprintf(fp, "\tMouseDy: %d\n", cmd->mouse_dy);
    }
}
DECL_FREE_FUNC(UserCmd) {}

// DataTables

static int get_send_prop_type(int type) {
    const int old_props[] = {
        SEND_PROP_INT, SEND_PROP_FLOAT,
        SEND_PROP_VECTOR3, SEND_PROP_STRING,
        SEND_PROP_ARRAY, SEND_PROP_DATATABLE
    };

    const int new_props[] = {
        SEND_PROP_INT, SEND_PROP_FLOAT, SEND_PROP_VECTOR3, SEND_PROP_VECTOR2,
        SEND_PROP_STRING, SEND_PROP_ARRAY, SEND_PROP_DATATABLE
    };

    const int* props;
    int props_size;
    if (demo_info.network_protocol <= 14) {
        props = old_props;
        props_size = sizeof(old_props) / sizeof(int);
    }
    else {
        props = new_props;
        props_size = sizeof(new_props) / sizeof(int);
    }

    if (type < props_size)
        return props[type];
    return SEND_PROP_INVALID;
}

static bool parse_send_prop(SendProp* thisptr, BitStream* bits) {
    thisptr->send_prop_type = get_send_prop_type(bits_read_bits(5, bits));
    if (thisptr->send_prop_type == SEND_PROP_INVALID) {
        warning("Invalid SendProp type in DataTables.\n");
        return false;
    }
    thisptr->send_prop_name = bits_read_str(bits);

    uint32_t send_prop_flag_bits = 0;
    if (demo_info.demo_protocol == 2) {
        send_prop_flag_bits = 11;
    }
    else if (demo_info.demo_protocol == 3) {
        send_prop_flag_bits = (demo_info.network_protocol <= 7) ? 13 : 16;
    }
    else if (demo_info.demo_protocol == 4) {
        send_prop_flag_bits = 19;
    }
    thisptr->send_prop_flags = bits_read_bits(send_prop_flag_bits, bits);

    if (demo_info.demo_protocol >= 4)
        thisptr->priority = bits_read_le_u8(bits);
    if (thisptr->send_prop_flags & (1 << 6)) {
        thisptr->exclude_dt_name = bits_read_str(bits);
    }
    else {
        if (thisptr->send_prop_type == SEND_PROP_DATATABLE) {
            thisptr->table_name = bits_read_str(bits);
        }
        else if (thisptr->send_prop_type == SEND_PROP_ARRAY) {
            thisptr->num_element = bits_read_bits(10, bits);
        }
        else {
            thisptr->low_value = bits_read_le_f32(bits);
            thisptr->high_value = bits_read_le_f32(bits);
            thisptr->num_bits = bits_read_bits((demo_info.network_protocol <= 14) ? 6 : 7, bits);
        }
    }
    return true;
}
static void print_send_prop(SendProp* thisptr, FILE* fp) {
    fprintf(fp, "\t\t\tSendPropType: %d\n", thisptr->send_prop_type);
    fprintf(fp, "\t\t\tSendPropName: %s\n", thisptr->send_prop_name);
    fprintf(fp, "\t\t\tSendPropFlags: %d\n", thisptr->send_prop_flags);
    if (demo_info.demo_protocol >= 4)
        fprintf(fp, "\t\t\tPriority: %d\n", thisptr->priority);
    if (thisptr->send_prop_flags & (1 << 6)) {
        fprintf(fp, "\t\t\tExcludeDtName: %s\n", thisptr->exclude_dt_name);
    }
    else {
        if (thisptr->send_prop_type == SEND_PROP_DATATABLE) {
            fprintf(fp, "\t\t\tTableName: %s\n", thisptr->table_name);
        }
        else if (thisptr->send_prop_type == SEND_PROP_ARRAY) {
            fprintf(fp, "\t\t\tNumElement: %d\n", thisptr->num_element);
        }
        else {
            fprintf(fp, "\t\t\tLowValue: %.3f\n", thisptr->low_value);
            fprintf(fp, "\t\t\tHighValue: %.3f\n", thisptr->high_value);
            fprintf(fp, "\t\t\tNumBits: %d\n", thisptr->num_bits);
        }
    }
}
static void free_send_prop(SendProp* thisptr) {
    free(thisptr->send_prop_name);
}

static bool parse_send_table(SendTable* thisptr, BitStream* bits) {
    thisptr->needs_decoder = bits_read_one_bit(bits);
    thisptr->net_table_name = bits_read_str(bits);
    thisptr->num_of_props = bits_read_bits((demo_info.network_protocol <= 7) ? 9 : 10, bits);
    thisptr->send_props.capacity = 0;
    thisptr->send_props.size = 0;
    if (thisptr->num_of_props == 0)
        return true;
    for (uint32_t i = 0; i < thisptr->num_of_props; i++) {
        SendProp prop;
        if (!parse_send_prop(&prop, bits))
            return false;
        vector_push(thisptr->send_props, prop);
    }
    vector_shrink(thisptr->send_props);
    return true;
}

static void print_send_table(const SendTable* thisptr, FILE* fp) {
    fprintf(fp, "\t\tNeedsDecoder: %s\n", thisptr->needs_decoder ? "true" : "false");
    fprintf(fp, "\t\tNetTableName: %s\n", thisptr->net_table_name);
    fprintf(fp, "\t\tNumOfProps: %d\n", thisptr->num_of_props);
    for (size_t i = 0; i < thisptr->send_props.size; i++) {
        fprintf(fp, "\t\tSendProp[%zd]\n", i);
        print_send_prop(&thisptr->send_props.data[i], fp);
    }
}

static void free_send_table(SendTable* thisptr) {
    free(thisptr->net_table_name);
    for (size_t i = 0; i < thisptr->send_props.size; i++) {
        free_send_prop(&thisptr->send_props.data[i]);
    }
}

static void parse_server_class_info(ServerClassInfo* thisptr, BitStream* bits) {
    thisptr->num_of_classes = bits_read_le_u16(bits);
    thisptr->class_id = bits_read_le_u16(bits);
    thisptr->class_name = bits_read_str(bits);
    thisptr->data_table_name = bits_read_str(bits);
}

static void print_server_class_info(const ServerClassInfo* thisptr, FILE* fp) {
    fprintf(fp, "\t\t\tNumOfClasses: %d\n", thisptr->num_of_classes);
    fprintf(fp, "\t\t\tClassId: %d\n", thisptr->class_id);
    fprintf(fp, "\t\t\tClassName: %s\n", thisptr->class_name);
    fprintf(fp, "\t\t\tDataTableName: %s\n", thisptr->data_table_name);
}

static void free_server_class_info(ServerClassInfo* thisptr) {
    free(thisptr->class_name);
    free(thisptr->data_table_name);
}

DECL_PARSE_FUNC(DataTables) {
    DECL_PTR(DataTables);
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (demo_info.parse_level >= 3) {
        bool error = false;
        ptr->send_tables.capacity = 0;
        ptr->send_tables.size = 0;
        while (bits_read_one_bit(bits)) {
            SendTable table;
            if (!parse_send_table(&table, bits)) {
                error = true;
                break;
            }
            vector_push(ptr->send_tables, table);
        }
        vector_shrink(ptr->send_tables);
        ptr->server_class_info.capacity = 0;
        ptr->server_class_info.size = 0;
        if (!error) {
            uint16_t classes = bits_read_le_u16(bits);
            for (uint16_t i = 0; i < classes; i++) {
                ServerClassInfo info;
                parse_server_class_info(&info, bits);
                vector_push(ptr->server_class_info, info);
            }
            vector_shrink(ptr->server_class_info);
        }
        if (error || bits->current > end_index) {
            warning("DataTables not parsed correctly.\n");
        }
        else if (end_index - bits->current >= 8) {
            warning("Had more than 1 byte remain after parsing DataTables.\n");
        }
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(DataTables) {
    const DECL_PTR(DataTables);
    if (demo_info.parse_level >= 3) {
        for (size_t i = 0; i < ptr->send_tables.size; i++) {
            fprintf(fp, "\tSendTable[%zd]\n", i);
            print_send_table(&ptr->send_tables.data[i], fp);
        }
        for (size_t i = 0; i < ptr->server_class_info.size; i++) {
            fprintf(fp, "\tServerClassInfo[%zd]\n", i);
            print_server_class_info(&ptr->server_class_info.data[i], fp);
        }
    }
}
DECL_FREE_FUNC(DataTables) {
    DECL_PTR(DataTables);
    if (demo_info.parse_level >= 3) {
        for (size_t i = 0; i < ptr->send_tables.size; i++) {
            free_send_table(&ptr->send_tables.data[i]);
        }
        for (size_t i = 0; i < ptr->server_class_info.size; i++) {
            free_server_class_info(&ptr->server_class_info.data[i]);
        }
    }
}

// Stop
DECL_PARSE_FUNC(Stop) {
    DECL_PTR(Stop);
    ptr->remaining_bytes = (bits->bit_size - bits->current) >> 3;
    return true;
}
DECL_PRINT_FUNC(Stop) {
    const DECL_PTR(Stop);
    fprintf(fp, "\t\tRemainingBytes: %zd\n", ptr->remaining_bytes);
}
DECL_FREE_FUNC(Stop) {}

// CustomData
DECL_PARSE_FUNC(CustomData) {
    DECL_PTR(CustomData);
    ptr->type = bits_read_le_u32(bits);
    uint32_t byte_size = ptr->size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if (ptr->type == 0) {
        RadialMouseMenuCallback* msg = &ptr->data.RadialMouseMenuCallback_message;
        msg->cursor_x = bits_read_le_u32(bits);
        msg->cursor_y = bits_read_le_u32(bits);

        msg->has_sar_message = (byte_size == 17);
        if (msg->has_sar_message) {
            msg->sar_message.id = bits_read_le_u8(bits);
            msg->sar_message.demo_checksum = bits_read_le_u32(bits);
            msg->sar_message.sar_checksum = bits_read_le_u32(bits);
        }
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(CustomData) {
    const DECL_PTR(CustomData);
    if (ptr->type == 0) {
        fprintf(fp, "\tRadialMouseMenuCallback (0)\n");
        const RadialMouseMenuCallback* msg = &ptr->data.RadialMouseMenuCallback_message;
        fprintf(fp, "\t\tCursorX: %d\n", msg->cursor_x);
        fprintf(fp, "\t\tCursorY: %d\n", msg->cursor_y);

        if (msg->has_sar_message) {
            fprintf(fp, "\t\tSourceAutoRecordMessage:\n");
            fprintf(fp, "\t\t\tId: %d\n", msg->sar_message.id);
            fprintf(fp, "\t\t\tDemoChecksum: %X\n", msg->sar_message.demo_checksum);
            fprintf(fp, "\t\t\tSAR_Checksum: %X\n", msg->sar_message.sar_checksum);
        }
    }
    else {
        fprintf(fp, "\tUnknownMessage (%d)\n", ptr->type);
        fprintf(fp, "\t\tSize: %d\n", ptr->size);
    }
}
DECL_FREE_FUNC(CustomData) {}

// StringTables
DECL_PARSE_FUNC(StringTables) {
    // DECL_PTR(StringTables);
    uint32_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    /*
    if (demo_info.parse_level >= 2) {
        // not implement
        ptr->size = byte_size;
        bits_skip(byte_size << 3, bits);
    }
    */
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(StringTables) {}
DECL_FREE_FUNC(StringTables) {}

// Invalid
DECL_PARSE_FUNC(Invalid) {
    return false;
}
DECL_PRINT_FUNC(Invalid) {}
DECL_FREE_FUNC(Invalid) {}

// settings
const DemoMessageSettings oe_msg_settings = {
    {MACRO_OE_MESSAGES(DECL_MSG_IN_NAME)},
    {MACRO_OE_MESSAGES(DECL_MSG_IN_ENUM)},
    {MACRO_OE_MESSAGES(DECL_MSG_IN_TABLE)}
};
const DemoMessageSettings ne_msg_settings = {
    {MACRO_NE_MESSAGES(DECL_MSG_IN_NAME)},
    {MACRO_NE_MESSAGES(DECL_MSG_IN_ENUM)},
    {MACRO_NE_MESSAGES(DECL_MSG_IN_TABLE)}
};