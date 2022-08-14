#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "demo.h"
#include "demo_info.h"
#include "bits.h"
#include "alloc.h"
#include "print.h"

const char* game_names[GAME_COUNT] = {
    "Dark Messiah of Might & Magic",
    "Half-Life 2 Old Engine",
    "Portal (3258)",
    "Portal (3420)",
    "Source Unpack (5135)",
    "Steampipe HL2/Portal",
    "Portal 2",
    "Unknown"
};

DemoInfo demo_info = { 0 };

Demo* new_demo(char* path) {
    Demo* demo = malloc_s(sizeof(Demo));
    char* file_name = strrchr(path, '/');
    file_name = file_name ? (file_name + 1) : path;
    demo->path = path;
    demo->file_name = file_name;
    demo->messages.size = 0;
    demo->messages.capacity = 0;
    demo->messages.data = NULL;
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

DemoTime demo_parse(Demo* demo, uint8_t parse_level, bool debug_mode) {
    demo_info.parse_level = parse_level;
    demo_info.debug_mode = debug_mode;

    BitStream* bits = bits_load_file(demo->path);
    DemoTime demo_time = { 0 };
    if (!bits) {
        error("Cannot open file %s.\n", demo->path);
        demo_time.state = MEASURED_ERROR;
        return demo_time;
    }

    int measured_ticks = 0;

    parse_header(demo, bits);
    if (parse_level == 0) {
        // only parse header
        demo_time.state = NOT_MEASURED;
        return demo_time;
    }

    demo_info.demo_protocol = demo->header.demo_protocol;
    demo_info.network_protocol = demo->header.network_protocol;

    demo_info.msg_settings = NULL;
    demo_info.net_msg_settings = NULL;

    // TODO: needs improvement
    demo_info.game = GAME_UNKNOWN;
    demo_info.has_tick_interval = false;
    demo_time.tick_interval = 0.0f;
    demo_info.MSSC = 1;
    switch (demo_info.demo_protocol) {
    case 2:
        if (demo_info.network_protocol == 7) {
            demo_info.game = DMOMM;
            demo_time.tick_interval = 0.015f;
        }
        demo_info.msg_settings = &portal_3420_msg_settings;
        demo_info.net_msg_settings = &oe_net_msg_settings;
        break;
    case 3:
        demo_info.net_msg_settings = &oe_net_msg_settings;
        switch (demo_info.network_protocol)
        {
        case 7:
            demo_info.game = HL2_OE;
            demo_time.tick_interval = 0.015f;
            demo_info.msg_settings = &portal_3420_msg_settings;
            break;
        case 11:
            demo_info.game = PORTAL_3258;
            demo_time.tick_interval = 0.015f;
            demo_info.msg_settings = &portal_3420_msg_settings;
            break;
        case 14:
            demo_info.game = PORTAL_3420;
            demo_time.tick_interval = 0.015f;
            demo_info.msg_settings = &portal_3420_msg_settings;
            break;
        case 15:
            demo_info.game = PORTAL_5135;
            demo_time.tick_interval = 0.015f;
            demo_info.msg_settings = &portal_5135_msg_settings;
            break;
        case 24:
            demo_info.game = PORTAL_1910503;
            demo_time.tick_interval = 0.015f;
            demo_info.msg_settings = &portal_5135_msg_settings;
            break;
        default:
            demo_info.msg_settings = &portal_5135_msg_settings;
            break;
        }
        break;
    case 4:
        if (demo_info.network_protocol == 2001) {
            demo_info.game = PORTAL_2;
            // 0x3C888889
            demo_time.tick_interval = 0.016666667f;
            demo_info.MSSC = 2;
        }
        demo_info.msg_settings = &ne_msg_settings;
        demo_info.net_msg_settings = &ne_net_msg_settings;
        break;
    default:
        error("Unsupported demo protocol: %d\n", demo_info.demo_protocol);
        bits_free(bits);
        demo_time.state = MEASURED_ERROR;
        return demo_time;
    }
    demo_time.game = demo_info.game;

    DemoMessageType type;
    uint32_t count = 0;
    do {
        count++;
        DemoMessage msg = { 0 };
        type = msg.type = bits_read_le_u8(bits);
        // Last byte is cut off in demos, use the previous byte
        int tick;
        if (bits->bit_size - bits->current >= 32) {
            tick = bits_read_le_u32(bits);
        }
        else {
            tick = bits_read_bits(24, bits);
            tick |= (tick << 8) & (0xff << 24);
        }
        msg.tick = tick;

        if (demo_info.MSSC > 1) {
            msg.slot = bits_read_le_u8(bits);
        }

        // parse message
        if (type < MESSAGE_COUNT) {
            const char* name = demo_info.msg_settings->names[type];
            debug("Parsing message type %s (%d) at %d.\n", name, type, count);
            bool success = demo_info.msg_settings->func_table[type].parse(&msg.data, bits);
            if (!success) {
                warning("Failed to parse message %s (%d) at %d.\n", name, type, count);
                demo_time.state = MEASURED_ERROR;
                break;
            }
            vector_push(demo->messages, msg);
        }
        else {
            error("Unexpected message type %d at %d.\n", type, count);
            demo_time.state = MEASURED_ERROR;
            break;
        }

        if (demo_info.msg_settings->enum_ids[type] == Packet_MSG && tick >= 0 && tick > measured_ticks) {
            measured_ticks = tick;
        }
    } while (bits->bit_size - bits->current >= 24); // would be 32 but the last byte is often cut off

    vector_shrink(demo->messages);
    bits_free(bits);

    demo_time.ticks = measured_ticks;
    demo_time.state = MEASURED_SUCCESS;
    if (demo_info.has_tick_interval) {
        debug("Found tick interval: %f\n", demo_info.tick_interval);
        demo_time.tick_interval = demo_info.tick_interval;
    }
    return demo_time;
}

void demo_verbose(const Demo* demo, FILE* fp) {
    if (!demo) return;

    fprintf(fp, "FileName: %s\n\n", demo->file_name);

    print_header(demo, fp);
    fprintf(fp, "\n");

    for (size_t i = 0; i < demo->messages.size; i++) {
        const DemoMessage* msg = &demo->messages.data[i];
        DemoMessageType type = msg->type;
        const char* name = demo_info.msg_settings->names[type];
        fprintf(fp, "[%d] %s (%d)\n", msg->tick, name, type);
        if (demo_info.MSSC > 1) {
            fprintf(fp, "\tSlot: %d\n", msg->slot);
        }
        demo_info.msg_settings->func_table[type].print(&msg->data, fp);
        fprintf(fp, "\n");
    }
}

void demo_gen_tas_script(const Demo* demo, FILE* fp) {
    fprintf(fp, "unpause;\n");

    int stop = demo_info.msg_settings->enum_ids[Stop_MSG];
    int console_cmd = demo_info.msg_settings->enum_ids[ConsoleCmd_MSG];
    int user_cmd = demo_info.msg_settings->enum_ids[UserCmd_MSG];

    for (size_t i = 0; i < demo->messages.size; i++) {
        const DemoMessage* msg = &demo->messages.data[i];
        int tick = msg->tick;
        if (tick > 0) {
            int type = msg->type;
            if (type == stop) {
                break;
            }
            if (type == console_cmd) {
                char* command = (char*)msg->data.ConsoleCmd_message.data;
                fprintf(fp, "_y_spt_afterframes %d \"%s;\";\n", tick, command);
            }
            else if (type == user_cmd) {
                const UserCmdInfo* cmd = &msg->data.UserCmd_message.data;
                fprintf(fp, "_y_spt_afterframes %d \"_y_spt_setangles %.8f %.8f;\";\n", tick, cmd->view_angles_x, cmd->view_angles_y);
            }
        }
    }
}

void demo_free(Demo* demo) {
    if (!demo) return;
    for (size_t i = 0; i < demo->messages.size; i++) {
        DemoMessage* msg = &demo->messages.data[i];
        demo_info.msg_settings->func_table[msg->type].free(&msg->data);
    }
    free(demo->messages.data);
    free(demo);
}