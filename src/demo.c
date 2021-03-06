#include "demo.h"
#include "bits.h"
#include <stdlib.h>
#include <string.h>

Demo* new_demo(char* path) {
    Demo* demo = malloc(sizeof(Demo));
    if(!demo) return NULL;
    char* file_name = strrchr(path ,'/');
    file_name = file_name ? (file_name + 1) : path;
    demo->path = path;
    demo->file_name = file_name;
    demo->message_len = 0;
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
    cmd->cmd = bits_read_le_u32(bits);
    size_t byte_size = bits_read_le_u32(bits);
    size_t end_index = bits->current + (byte_size << 3);
    if((cmd->has_command_number = bits_read_one_bit(bits)))
        cmd->command_number = bits_read_le_u32(bits);

    if((cmd->has_tick_count = bits_read_one_bit(bits)))
        cmd->tick_count = bits_read_le_u32(bits);

    if((cmd->has_view_angles_x = bits_read_one_bit(bits)))
        cmd->view_angles_x = bits_read_le_f32(bits);
    if((cmd->has_view_angles_y = bits_read_one_bit(bits)))
        cmd->view_angles_y = bits_read_le_f32(bits);
    if((cmd->has_view_angles_z = bits_read_one_bit(bits)))
        cmd->view_angles_z = bits_read_le_f32(bits);

    if((cmd->has_forward_move = bits_read_one_bit(bits)))
        cmd->forward_move = bits_read_le_f32(bits);
    if((cmd->has_side_move = bits_read_one_bit(bits)))
        cmd->side_move = bits_read_le_f32(bits);
    if((cmd->has_up_move = bits_read_one_bit(bits)))
        cmd->up_move = bits_read_le_f32(bits);

    if((cmd->has_buttons = bits_read_one_bit(bits)))
        cmd->buttons = bits_read_le_u32(bits);
    
    if((cmd->has_impulse = bits_read_one_bit(bits)))
        cmd->impulse = bits_read_le_u8(bits);
    
    if((cmd->has_weapon_select = bits_read_one_bit(bits))) {
        cmd->weapon_select = bits_read_bits(11, bits);
        if((cmd->has_weapon_subtype = bits_read_one_bit(bits)))
            cmd->weapon_subtype = bits_read_bits(6, bits);
    }
    
    if((cmd->has_mouse_dx = bits_read_one_bit(bits)))
        cmd->mouse_dx = bits_read_le_u16(bits);
    if((cmd->has_mouse_dy = bits_read_one_bit(bits)))
        cmd->mouse_dy = bits_read_le_u16(bits);

    if(bits->current > end_index)
        fprintf(stderr, "[WARNING] Usercmd not parse correctly.\n");
    bits->current = end_index;
}

int demo_parse(Demo* demo) {
    FILE* fp = fopen(demo->path, "rb");
    if(!fp) {
        fprintf(stderr, "[ERROR] Cannot open file %s.\n", demo->path);
        return -1;
    }
    size_t file_size;
    fseek(fp, 0L, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    BitStream* bits = bits_init(file_size, fp);
    fclose(fp);

    int measured_ticks = 0;
    demo->messages = NULL;

    parse_header(demo, bits);

    demo->message_len = 0;
    int i = 0;
    while(1) {
        demo->message_len++;
        demo->messages = realloc(demo->messages, sizeof(DemoMessage) * (i + 1));

        demo->messages[i].type = bits_read_le_u8(bits);
        demo->messages[i].tick = bits_read_le_u32(bits);
        if(demo->messages[i].tick >= 0 && demo->messages[i].tick > measured_ticks)
            measured_ticks = demo->messages[i].tick;

        demo->messages[i].data = NULL;

        switch (demo->messages[i].type) {
        case SIGN_ON:
            {
                size_t byte_size;
                bits->current += (76 + 8) << 3;
                byte_size = bits_read_le_u32(bits);
                bits->current += byte_size << 3;
            }
            break;
        case PACKET:
            {
                size_t byte_size;
                demo->messages[i].data = malloc(sizeof(CmdInfo));
                parse_cmd_info((CmdInfo*)demo->messages[i].data, bits);
                bits->current += 8 << 3;
                byte_size = bits_read_le_u32(bits);
                bits->current += byte_size << 3;
            }
            break;

        case SYNC_TICK:
            break;

        case CONSOLECMD:
            {
                size_t byte_size = bits_read_le_u32(bits);
                demo->messages[i].data = malloc(byte_size);
                bits_read_bytes((char*)demo->messages[i].data, byte_size, bits);
            }
            
            break;

        case USERCMD:
            demo->messages[i].data = malloc(sizeof(UserCmd));
            parse_usercmd((UserCmd*)demo->messages[i].data, bits);
            break;

        case DATA_TABLES:
            {
                uint32_t byte_size = bits_read_le_u32(bits);
                bits->current += byte_size << 3;
                break;
            }
            break;

        case STOP:
            bits_free(bits);
            return measured_ticks;
        
        case STRING_TABLES:
            {
                size_t size = bits_read_le_u32(bits);
                bits->current += size << 3;
                break;
            }
            break;

        default:
			fprintf(stderr, "[ERROR] Unexpected message type %d at message %d\n", demo->messages[i].type, i);
            return -1;
        }
		i++;
    }
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
    if(cmd->has_command_number)
        fprintf(fp, "\tCommandNumber: %d\n", cmd->command_number);
    if(cmd->has_tick_count)
        fprintf(fp, "\tTickCount: %d\n", cmd->tick_count);
    if(cmd->has_view_angles_x)
        fprintf(fp, "\tViewAnglesX: %.3f\n", cmd->view_angles_x);
    if(cmd->has_view_angles_y)
        fprintf(fp, "\tViewAnglesY: %.3f\n", cmd->view_angles_y);
    if(cmd->has_view_angles_z)
        fprintf(fp, "\tViewAnglesZ: %.3f\n", cmd->view_angles_z);
    if(cmd->has_forward_move)
        fprintf(fp, "\tForwardMove: %.3f\n", cmd->forward_move);
    if(cmd->has_side_move)
        fprintf(fp, "\tSideMove: %.3f\n", cmd->side_move);
    if(cmd->has_up_move)
        fprintf(fp, "\tUpMove: %.3f\n", cmd->up_move);
    if(cmd->has_buttons)
        fprintf(fp, "\tButtons: %d\n", cmd->buttons);
    if(cmd->has_impulse)
        fprintf(fp, "\tImpulse: %d\n", cmd->impulse);
    if(cmd->has_weapon_select) {
        fprintf(fp, "\tWeaponSelect: %d\n", cmd->weapon_select);
        if(cmd->has_weapon_subtype)
            fprintf(fp, "\tWeaponSubtype: %d\n", cmd->weapon_subtype);
    }
    if(cmd->has_mouse_dx)
        fprintf(fp, "\tMouseDx: %d\n", cmd->mouse_dx);
    if(cmd->has_mouse_dy)
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
    if(!demo) return;
    
    fprintf(fp, "FileName: %s\n\n", demo->file_name);

    print_header(demo, fp);
    fprintf(fp, "\n");

	size_t i = 0;
	while(i < demo->message_len) {
        int type = demo->messages[i].type;
        fprintf(fp, "[%d] ", demo->messages[i].tick);
        switch(type) {
        case SIGN_ON:
            fprintf(fp, "SignOn\n");
            break;
        case PACKET:
            fprintf(fp, "Packet\n");
            print_cmd_info((CmdInfo*)demo->messages[i].data, fp);
            break;
        case SYNC_TICK:
            fprintf(fp, "SyncTick\n");
            break;
        case CONSOLECMD:
            fprintf(fp, "ConsoleCmd\n");
            fprintf(fp, "\tData: %s\n", (char*)demo->messages[i].data);
            break;
        case USERCMD:
            print_usercmd((UserCmd*)demo->messages[i].data, fp);
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
		i++;
    }
}

void demo_gen_tas_script(const Demo* demo, FILE* fp) {
    size_t i = 0;
    fprintf(fp, "unpause;\n");
	while(i < demo->message_len) {
        int tick = demo->messages[i].tick;
        if(tick > 0) {
            int type = demo->messages[i].type;
            if(type == STOP) {
                break;
            }
            if(type == CONSOLECMD) {
                char* command = (char*)demo->messages[i].data;
                fprintf(fp, "_y_spt_afterframes %d \"%s;\";\n", tick, command);
            } else if(type == USERCMD) {
                UserCmd* cmd = (UserCmd*)demo->messages[i].data;
                fprintf(fp, "_y_spt_afterframes %d \"_y_spt_setangles %.8f %.8f;\";\n", tick, cmd->view_angles_x, cmd->view_angles_y);
            }
        }
		i++;
    }
}

void demo_free(Demo* demo) {
    if(!demo) return;

    for(size_t i = 0; i < demo->message_len; i++) {
        if(demo->messages[i].data)
            free(demo->messages[i].data);
    }
	free(demo->messages);
    free(demo);
}