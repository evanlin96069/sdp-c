#include "demo.h"

#include <stdlib.h>
#include <string.h>

#include "bits.h"
#include "alloc.h"

Demo* new_demo(char* path) {
    Demo* demo = malloc_s(sizeof(Demo));
    char* file_name = strrchr(path, '/');
    file_name = file_name ? (file_name + 1) : path;
    demo->path = path;
    demo->file_name = file_name;
    demo->messages = NULL;
    return demo;
}

static void parse_header(Demo* demo, BitStream* bits) {
    bits_read_bytes(demo->header.demo_file_stamp, 8, bits);
    demo->header.demo_protocol = bits_read_le_u32(bits);
    demo->header.network_protocol = bits_read_le_u32(bits);
    bits_read_bytes(demo->header.server_name, 260, bits);
    bits_read_bytes(demo->header.client_name, 260, bits);
    bits_read_bytes(demo->header.map_name, 260, bits);
    bits_read_bytes(demo->header.game_dir, 260, bits);
    demo->header.play_back_time = bits_read_le_f32(bits);
    demo->header.play_back_ticks = bits_read_le_u32(bits);
    demo->header.play_back_frames = bits_read_le_u32(bits);
    demo->header.sign_on_length = bits_read_le_u32(bits);
}

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

static void parse_usercmd(UserCmd* cmd, BitStream* bits) {
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

int demo_parse(Demo* demo, bool quick_mode) {
    BitStream* bits = bits_load_file(demo->path);
    if (!bits) {
        fprintf(stderr, "[ERROR] Cannot open file %s.\n", demo->path);
        return -1;
    }

    int measured_ticks = 0;

    DemoMessage* tail = NULL;
    demo->messages = NULL;

    parse_header(demo, bits);

    bool stop_reading = false;
    int count = 0;
    while (!stop_reading) {
        count++;
        DemoMessage* msg = malloc_s(sizeof(DemoMessage));
        msg->next = NULL;
        msg->data = NULL;

        int type = msg->type = bits_read_le_u8(bits);
        // Last byte is cut off in demos, use the previous byte
        int tick;
        if (bits->bit_size - bits->current >= 32) {
            tick = bits_read_le_u32(bits);
        }
        else {
            tick = bits_read_bits(24, bits);
            tick |= (tick << 8) & (0xff << 24);
        }
        msg->tick = tick;

        if (tick >= 0 && tick > measured_ticks) {
            measured_ticks = tick;
        }

        switch (type) {
        case SIGN_ON:
        {
            size_t byte_size;
            // PacketInfo(76)
            bits_skip(76 << 3, bits);
            // InSequence(4) + OutSequence(4)
            bits_skip(8 << 3, bits);
            byte_size = bits_read_le_u32(bits);
            bits_skip(byte_size << 3, bits);
        }
        break;
        case PACKET:
        {
            size_t byte_size;
            msg->data = malloc_s(sizeof(CmdInfo));
            if (!quick_mode) {
                parse_cmd_info((CmdInfo*)msg->data, bits);
            }
            else {
                // PacketInfo(76)
                bits_skip(76 << 3, bits);
            }
            // InSequence(4) + OutSequence(4)
            bits_skip(8 << 3, bits);
            byte_size = bits_read_le_u32(bits);
            bits_skip(byte_size << 3, bits);
        }
        break;

        case SYNC_TICK:
            break;

        case CONSOLECMD:
        {
            size_t byte_size = bits_read_le_u32(bits);
            if (!quick_mode) {
                msg->data = malloc_s(byte_size);
                bits_read_bytes((char*)msg->data, byte_size, bits);
            }
            else {
                bits_skip(byte_size << 3, bits);
            }
        }
        break;

        case USERCMD:
        {
            uint32_t cmd = bits_read_le_u32(bits);
            size_t byte_size = bits_read_le_u32(bits);
            size_t end_index = bits->current + (byte_size << 3);
            if (!quick_mode) {
                msg->data = malloc_s(sizeof(UserCmd));
                ((UserCmd*)msg->data)->cmd = cmd;
                parse_usercmd((UserCmd*)msg->data, bits);
                if (bits->current > end_index) {
                    fprintf(stderr, "[WARNING] Usercmd not parse correctly.\n");
                }
            }
            bits->current = end_index;
            bits_fetch(bits);
        }
        break;

        case DATA_TABLES:
        {
            uint32_t byte_size = bits_read_le_u32(bits);
            bits_skip(byte_size << 3, bits);
        }
        break;

        case STOP:
            bits_free(bits);
            stop_reading = true;
            break;

        case STRING_TABLES:
        {
            size_t byte_size = bits_read_le_u32(bits);
            bits_skip(byte_size << 3, bits);
        }
        break;

        default:
            fprintf(stderr, "[ERROR] Unexpected type %d at message %d.\n", type, count);
            stop_reading = true;
        }

        if (!demo->messages) {
            demo->messages = msg;
            tail = msg;
        }
        else {
            tail->next = msg;
            tail = msg;
        }
    }
    return measured_ticks;
}

static void print_header(const Demo* demo, FILE* fp) {
    const DemoHeader* header = &demo->header;
    fprintf(fp, "[Header]\n");
    fprintf(fp, "DemoFileStamp: %s\n", header->demo_file_stamp);
    fprintf(fp, "DemoProtocol: %d\n", header->demo_protocol);
    fprintf(fp, "NetworkProtocol: %d\n", header->network_protocol);
    fprintf(fp, "ServerName: %s\n", header->server_name);
    fprintf(fp, "ClientName: %s\n", header->client_name);
    fprintf(fp, "MapName: %s\n", header->map_name);
    fprintf(fp, "GameDirectory: %s\n", header->game_dir);
    fprintf(fp, "PlayBackTime: %.3f\n", header->play_back_time);
    fprintf(fp, "PlayBackTicks: %d\n", header->play_back_ticks);
    fprintf(fp, "PlayBackFrames: %d\n", header->play_back_frames);
    fprintf(fp, "SignOnLength: %d\n", header->sign_on_length);
}

static void print_usercmd(const UserCmd* cmd, FILE* fp) {
    fprintf(fp, "UserCmd\n");
    fprintf(fp, "\tCmd: %d\n", cmd->cmd);
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

static void print_cmd_info(const CmdInfo* info, FILE* fp) {
    fprintf(fp, "\tFlags: %d\n", info->flags);
    fprintf(fp, "\tViewOrigin: (%.3f, %.3f, %.3f)\n", info->view_origin[0], info->view_origin[1], info->view_origin[2]);
    fprintf(fp, "\tViewAngles: (%.3f, %.3f, %.3f)\n", info->view_angles[0], info->view_angles[1], info->view_angles[2]);
    fprintf(fp, "\tLocalViewAngles: (%.3f, %.3f, %.3f)\n", info->local_view_angles[0], info->local_view_angles[1], info->local_view_angles[2]);
    fprintf(fp, "\tViewOrigin2: (%.3f, %.3f, %.3f)\n", info->view_origin2[0], info->view_origin2[1], info->view_origin2[2]);
    fprintf(fp, "\tViewAngles2: (%.3f, %.3f, %.3f)\n", info->view_angles2[0], info->view_angles2[1], info->view_angles2[2]);
    fprintf(fp, "\tLocalViewAngles2: (%.3f, %.3f, %.3f)\n", info->local_view_angles2[0], info->local_view_angles2[1], info->local_view_angles2[2]);
}

void demo_verbose(const Demo* demo, FILE* fp) {
    if (!demo) return;

    fprintf(fp, "FileName: %s\n\n", demo->file_name);

    print_header(demo, fp);
    fprintf(fp, "\n");

    if (!demo->messages) return;

    for (DemoMessage* msg = demo->messages; msg; msg = msg->next) {
        int type = msg->type;
        fprintf(fp, "[%d] ", msg->tick);
        switch (type) {
        case SIGN_ON:
            fprintf(fp, "SignOn\n");
            break;
        case PACKET:
            fprintf(fp, "Packet\n");
            print_cmd_info((CmdInfo*)msg->data, fp);
            break;
        case SYNC_TICK:
            fprintf(fp, "SyncTick\n");
            break;
        case CONSOLECMD:
            fprintf(fp, "ConsoleCmd\n");
            fprintf(fp, "\tData: %s\n", (char*)msg->data);
            break;
        case USERCMD:
            print_usercmd((UserCmd*)msg->data, fp);
            break;
        case DATA_TABLES:
            fprintf(fp, "DataTables\n");
            break;
        case STOP:
            fprintf(fp, "Stop\n");
            return;
        case STRING_TABLES:
            fprintf(fp, "StringTables\n");
            break;
        }
        fprintf(fp, "\n");
    }
}

void demo_gen_tas_script(const Demo* demo, FILE* fp) {
    fprintf(fp, "unpause;\n");
    for (DemoMessage* msg = demo->messages; msg; msg = msg->next) {
        int tick = msg->tick;
        if (tick > 0) {
            int type = msg->type;
            if (type == STOP) {
                break;
            }
            if (type == CONSOLECMD) {
                char* command = (char*)msg->data;
                fprintf(fp, "_y_spt_afterframes %d \"%s;\";\n", tick, command);
            }
            else if (type == USERCMD) {
                UserCmd* cmd = (UserCmd*)msg->data;
                fprintf(fp, "_y_spt_afterframes %d \"_y_spt_setangles %.8f %.8f;\";\n", tick, cmd->view_angles_x, cmd->view_angles_y);
            }
        }
    }
}

void demo_free(Demo* demo) {
    if (!demo) return;
    DemoMessage* msg = demo->messages;
    while (msg) {
        if (msg->data)
            free(msg->data);
        DemoMessage* tmp = msg;
        msg = msg->next;
        free(tmp);
    }
    demo->messages = NULL;
    free(demo);
}