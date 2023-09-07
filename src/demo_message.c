#include "demo_message.h"
#include "demo_info.h"
#include "utils/alloc.h"
#include "utils/print.h"
#include "utils/indent_writer.h"

#define DECL_PARSE_FUNC(type) static bool PARSE_FUNC_NAME(type)(DemoMessageData* thisptr, BitStream* bits)
#define DECL_PRINT_FUNC(type) static void PRINT_FUNC_NAME(type)(const DemoMessageData* thisptr)
#define DECL_FREE_FUNC(type) static void FREE_FUNC_NAME(type)(DemoMessageData* thisptr)

// Packet

static inline void parse_cmd_info(CmdInfo* info, BitStream* bits) {
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
        uint32_t net_msg_type_bits = (demo_info.network_protocol <= 14) ? 5 : 6;
        while (end_index - bits->current >= net_msg_type_bits) {
            count++;
            NetSvcMessage msg = { 0 };
            // parse net message
            NetSvcMessageType type = msg.type = bits_read_bits(net_msg_type_bits, bits);
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
        if (ptr->data.size)
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

DECL_PRINT_FUNC(Packet) {
    const DECL_PTR(Packet);
    for (int i = 0; i < demo_info.MSSC; i++) {
        write_line("PacketInfo[%d]:\n", i);
        g_writer.indent++;
        write_int("Flags", ptr->packet_info[i].flags);
        write_vec3("ViewOrigin", ptr->packet_info[i].view_origin);
        write_vec3("ViewAngles", ptr->packet_info[i].view_angles);
        write_vec3("LocalViewAngles", ptr->packet_info[i].local_view_angles);
        write_vec3("ViewOrigin2", ptr->packet_info[i].view_origin2);
        write_vec3("ViewAngles2", ptr->packet_info[i].view_angles2);
        write_vec3("LocalViewAngles2", ptr->packet_info[i].local_view_angles2);
        g_writer.indent--;
    }
    write_int("InSequence", ptr->in_sequence);
    write_int("OutSequence", ptr->out_sequence);

    if (demo_info.parse_level >= 2) {
        write_line("NET/SVC-Messages:\n");
        g_writer.indent++;
        for (size_t i = 0; i < ptr->data.size; i++) {
            NetSvcMessage* msg = &ptr->data.data[i];
            NetSvcMessageType type = msg->type;
            const char* name = demo_info.net_msg_settings->names[type];
            write_line("%s (%d)\n", name, type);
            g_writer.indent++;
            demo_info.net_msg_settings->func_table[type].print(&msg->data);
            g_writer.indent--;
        }
        g_writer.indent--;
    }
}
DECL_FREE_FUNC(Packet) {
    DECL_PTR(Packet);
    if (demo_info.parse_level >= 2 && ptr->data.size) {
        for (size_t i = 0; i < ptr->data.size; i++) {
            NetSvcMessage* msg = &ptr->data.data[i];
            demo_info.net_msg_settings->func_table[msg->type].free(&msg->data);
        }
        free(ptr->data.data);
    }
}

// SignOn
DECL_PARSE_FUNC(SignOn) {
    return parse_Packet(thisptr, bits);
}
DECL_PRINT_FUNC(SignOn) {
    print_Packet(thisptr);
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
    write_string("Data", (char*)ptr->data);
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
        write_int("CommandNumber", cmd->command_number);
    if (cmd->has_tick_count)
        write_int("TickCount", cmd->tick_count);
    if (cmd->has_view_angles_x)
        write_float("ViewAnglesX", cmd->view_angles_x);
    if (cmd->has_view_angles_y)
        write_float("ViewAnglesY", cmd->view_angles_y);
    if (cmd->has_view_angles_z)
        write_float("ViewAnglesZ", cmd->view_angles_z);
    if (cmd->has_forward_move)
        write_float("ForwardMove", cmd->forward_move);
    if (cmd->has_side_move)
        write_float("SideMove", cmd->side_move);
    if (cmd->has_up_move)
        write_float("UpMove", cmd->up_move);
    if (cmd->has_buttons)
        write_int("Buttons", cmd->buttons);
    if (cmd->has_impulse)
        write_int("Impulse", cmd->impulse);

    if (cmd->has_weapon_select) {
        write_int("WeaponSelect", cmd->weapon_select);
        if (cmd->has_weapon_subtype)
            write_int("WeaponSubtype", cmd->weapon_subtype);
        if (demo_info.game == DMOMM)
            write_int("UnknownWeaponFlags", cmd->mm_unknown_weapon_flags);
    }

    if (demo_info.game == DMOMM) {
        if (cmd->has_unknown_b_1) {
            write_bool("Unknown_b_1", cmd->unknown_b_1);
            write_bool("Unknown_b_2", cmd->unknown_b_2);
        }
        if (cmd->has_unknown_u11)
            write_int("Unknown_u11", cmd->unknown_u11);
        if (cmd->has_mm_move_item_from_slot)
            write_int("MoveItemFromSlot", cmd->mm_move_item_from_slot);
        if (cmd->has_mm_move_item_to_slot)
            write_int("MoveItemToSlot", cmd->mm_move_item_to_slot);
        if (cmd->has_mm_stealth)
            write_float("Stealth", cmd->mm_stealth);
        if (cmd->has_mm_use_item_id) {
            write_int("UseItemId", cmd->mm_use_item_id);
            write_bool("UnknownItemFlag", cmd->mm_unknown_item_flag);
        }
        if (cmd->has_unknown_i6)
            write_int("Unknown_i6", cmd->unknown_i6);
        if (cmd->has_mm_upgrade_skill_type)
            write_int("UpgradeSkillType", cmd->mm_upgrade_skill_type);

        if (cmd->has_mouse_dx)
            write_int("MouseDx", cmd->mouse_dx);
        if (cmd->has_mouse_dy)
            write_int("MouseDy", cmd->mouse_dy);

        // ???
        if (cmd->has_unknown_i16)
            write_int("Unknown_i16", cmd->unknown_i16);

        if (cmd->has_mm_lean_move)
            write_float("LeanMove", cmd->mm_lean_move);

        if (cmd->has_mm_sprint)
            write_bool("Sprint", cmd->mm_sprint);
        if (cmd->has_mm_unknown_action_2)
            write_bool("UnknownAction_2", cmd->mm_unknown_action_2);
        if (cmd->has_mm_kick)
            write_bool("Kick", cmd->mm_kick);
        if (cmd->has_mm_unknown_action_4)
            write_bool("UnknownAction_4", cmd->mm_unknown_action_4);
        if (cmd->has_mm_unknown_action_5)
            write_bool("UnknownAction_5", cmd->mm_unknown_action_5);
        if (cmd->has_mm_shwo_charsheet)
            write_bool("ShowCharsheet", cmd->mm_shwo_charsheet);
        if (cmd->has_mm_unknown_action_7)
            write_bool("UnknownAction_7", cmd->mm_unknown_action_7);
        if (cmd->has_mm_show_inventory_belt)
            write_bool("ShowInventoryBelt", cmd->mm_show_inventory_belt);
        if (cmd->has_mm_show_inventory_belt_select)
            write_bool("ShowInventoryBeltSelect", cmd->mm_show_inventory_belt_select);
        if (cmd->has_mm_hide_inventory_belt_select)
            write_bool("HideInventoryBeltSelect", cmd->mm_hide_inventory_belt_select);
        if (cmd->has_mm_show_objectives)
            write_bool("ShowObjectives", cmd->mm_show_objectives);
        if (cmd->has_mm_hide_objectives)
            write_bool("HideObjectives", cmd->mm_hide_objectives);

        if (cmd->has_mm_exit_book_id)
            write_int("ExitBookId", cmd->mm_exit_book_id);
        if (cmd->has_mm_xana)
            write_bool("Xana", cmd->mm_xana);
    }
    else {
        if (cmd->has_mouse_dx)
            write_int("MouseDx", cmd->mouse_dx);
        if (cmd->has_mouse_dy)
            write_int("MouseDy", cmd->mouse_dy);
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
static void print_send_prop(SendProp* thisptr) {
    write_int("SendPropType", thisptr->send_prop_type);
    write_string("SendPropName", thisptr->send_prop_name);
    write_int("SendPropFlags", thisptr->send_prop_flags);
    if (demo_info.demo_protocol >= 4)
        write_int("Priority: %d\n", thisptr->priority);
    if (thisptr->send_prop_flags & (1 << 6)) {
        write_string("ExcludeDtName", thisptr->exclude_dt_name);
    }
    else {
        if (thisptr->send_prop_type == SEND_PROP_DATATABLE) {
            write_string("TableName", thisptr->table_name);
        }
        else if (thisptr->send_prop_type == SEND_PROP_ARRAY) {
            write_int("NumElement", thisptr->num_element);
        }
        else {
            write_float("LowValue", thisptr->low_value);
            write_float("HighValue", thisptr->high_value);
            write_int("NumBits", thisptr->num_bits);
        }
    }
}
static inline void free_send_prop(SendProp* thisptr) {
    free(thisptr->send_prop_name);
    if (thisptr->send_prop_flags & (1 << 6)) {
        free(thisptr->exclude_dt_name);
    }
    else if (thisptr->send_prop_type == SEND_PROP_DATATABLE) {
        free(thisptr->table_name);
    }
}

static bool parse_send_table(SendTable* thisptr, BitStream* bits) {
    thisptr->needs_decoder = bits_read_one_bit(bits);
    thisptr->net_table_name = bits_read_str(bits);
    thisptr->num_of_props = bits_read_bits((demo_info.network_protocol <= 7) ? 9 : 10, bits);
    if (thisptr->num_of_props == 0)
        return true;
    thisptr->send_props = malloc_s(thisptr->num_of_props * sizeof(SendProp));
    for (uint32_t i = 0; i < thisptr->num_of_props; i++) {
        if (!parse_send_prop(&thisptr->send_props[i], bits))
            return false;
    }
    return true;
}

static void print_send_table(const SendTable* thisptr) {
    write_bool("NeedsDecoder", thisptr->needs_decoder);
    write_string("NetTableName", thisptr->net_table_name);
    write_int("NumOfProps", thisptr->num_of_props);
    for (size_t i = 0; i < thisptr->num_of_props; i++) {
        write_line("SendProp[%zd]\n", i);
        g_writer.indent++;
        print_send_prop(&thisptr->send_props[i]);
        g_writer.indent--;
    }
}

static void free_send_table(SendTable* thisptr) {
    free(thisptr->net_table_name);
    if (thisptr->num_of_props == 0)
        return;
    for (size_t i = 0; i < thisptr->num_of_props; i++) {
        free_send_prop(&thisptr->send_props[i]);
    }
    free(thisptr->send_props);
}

static inline void parse_server_class_info(ServerClassInfo* thisptr, BitStream* bits) {
    thisptr->num_of_classes = bits_read_le_u16(bits);
    thisptr->class_id = bits_read_le_u16(bits);
    thisptr->class_name = bits_read_str(bits);
    thisptr->data_table_name = bits_read_str(bits);
}

static inline void print_server_class_info(const ServerClassInfo* thisptr) {
    write_int("NumOfClasses", thisptr->num_of_classes);
    write_int("ClassId", thisptr->class_id);
    write_string("ClassName", thisptr->class_name);
    write_string("DataTableName", thisptr->data_table_name);
}

static inline void free_server_class_info(ServerClassInfo* thisptr) {
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
        if (ptr->send_tables.size)
            vector_shrink(ptr->send_tables);
        if (!error) {
            ptr->classes = bits_read_le_u16(bits);
            if (ptr->classes) {
                ptr->server_class_info = malloc(ptr->classes * sizeof(ServerClassInfo));
                for (uint16_t i = 0; i < ptr->classes; i++) {
                    parse_server_class_info(&ptr->server_class_info[i], bits);
                }
            }
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
            write_line("SendTable[%zd]\n", i);
            g_writer.indent++;
            print_send_table(&ptr->send_tables.data[i]);
            g_writer.indent--;
        }
        for (size_t i = 0; i < ptr->classes; i++) {
            write_line("ServerClassInfo[%zd]\n", i);
            g_writer.indent++;
            print_server_class_info(&ptr->server_class_info[i]);
            g_writer.indent--;
        }
    }
}
DECL_FREE_FUNC(DataTables) {
    DECL_PTR(DataTables);
    if (demo_info.parse_level >= 3) {
        for (size_t i = 0; i < ptr->send_tables.size; i++) {
            free_send_table(&ptr->send_tables.data[i]);
        }
        if (ptr->send_tables.size)
            free(ptr->send_tables.data);
        if (ptr->classes) {
            for (size_t i = 0; i < ptr->classes; i++) {
                free_server_class_info(&ptr->server_class_info[i]);
            }
            free(ptr->server_class_info);
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
    write_size("RemainingBytes", ptr->remaining_bytes);
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

        msg->has_sar_message = (end_index - bits->current >= 8);
        if (msg->has_sar_message) {
            msg->sar_message.id = bits_read_le_u8(bits);
            if (msg->sar_message.id == 0xFF) {
                // v1
                msg->sar_message.data.checksum.demo = bits_read_le_u32(bits);
                msg->sar_message.data.checksum.sar_dll = bits_read_le_u32(bits);
            }
            else if (msg->sar_message.id == 0xFE) {
                // v2
                msg->sar_message.data.signature = ((uint64_t)bits_read_le_u32(bits) << 32) | bits_read_le_u32(bits);
            }

        }
    }
    bits_setpos(end_index, bits);
    return true;
}
DECL_PRINT_FUNC(CustomData) {
    const DECL_PTR(CustomData);
    if (ptr->type == 0) {
        write_line("RadialMouseMenuCallback (0)\n");
        const RadialMouseMenuCallback* msg = &ptr->data.RadialMouseMenuCallback_message;
        g_writer.indent++;
        write_int("CursorX", msg->cursor_x);
        write_int("CursorY", msg->cursor_y);

        if (msg->has_sar_message) {
            write_line("SourceAutoRecordMessage:\n");
            g_writer.indent++;
            if (msg->sar_message.id == 0xFF) {
                write_line("DemoChecksum: %X\n", msg->sar_message.data.checksum.demo);
                write_line("SAR_Checksum: %X\n", msg->sar_message.data.checksum.sar_dll);
            }
            else if (msg->sar_message.id == 0xFE) {
                write_line("Signature: %lX\n", msg->sar_message.data.signature);
            }
            g_writer.indent--;
        }
        g_writer.indent--;
    }
    else {
        write_line("UnknownMessage (%d)\n", ptr->type);
        g_writer.indent++;
        write_int("Size", ptr->size);
        g_writer.indent--;
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
