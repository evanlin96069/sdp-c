#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "demo.h"
#include "demo_info.h"
#include "version.h"
#include "../src/utils/print.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#define help(arg_name, help_info) printf("  %-24s %s\n", arg_name, help_info)

enum {
    QUICK_MODE,
    DUMP_MODE,
    TAS_MODE
};

static SvcServerInfo* get_SvcServerInfo(const Demo* demo) {
    int signon_type = demo_info.msg_settings->enum_ids[SignOn_MSG];
    int server_info = demo_info.net_msg_settings->enum_ids[SvcServerInfo_MSG];
    for(size_t i = 0; i < demo->messages.size; i++) {
        const DemoMessage* msg = &demo->messages.data[i];
        if (msg->type == signon_type) {
            const Packet* packet = &msg->data.Packet_message;
            for (size_t j = 0; j < packet->data.size; j++) {
                if (packet->data.data[j].type == server_info) {
                    return &packet->data.data[j].data.SvcServerInfo_message;
                }
            }
        }
    }
    return NULL;
}

static void print_quick_mode(const Demo* demo, int result) {
    const DemoHeader* header = &demo->header;
    const SvcServerInfo* server_info = NULL;
    if (demo->parse_level >= 2) {
        server_info = get_SvcServerInfo(demo);
    }

    printf("FileName:           %s\n", demo->file_name);
    printf("DemoProtocol:       %d\n", header->demo_protocol);
    printf("NetworkProtocol:    %d\n", header->network_protocol);
    printf("ServerName:         %s\n", header->server_name);
    printf("ClientName:         %s\n", header->client_name);
    printf("MapName:            %s\n", header->map_name);
    printf("GameDirectory:      %s\n", header->game_dir);
    printf("PlayBackTime:       %.3f\n", header->play_back_time);
    printf("PlayBackTicks:      %d\n", header->play_back_ticks);
    printf("PlayBackFrames:     %d\n", header->play_back_frames);
    if (server_info) {
        printf("ServerCount:        %d\n", server_info->server_count);
    }

    if (result != NOT_MEASURED) {
#if 0
        // Print Tags
        int console_cmd = demo_info.msg_settings->enum_ids[ConsoleCmd_MSG];
        for(size_t i = 0; i < demo->messages.size; i++) {
            const DemoMessage* msg = &demo->messages.data[i];
            if (msg->type == console_cmd) {
                // TODO: search string
            }
        }
#endif
        // Print time
        printf(SET_FG(CYN));
        printf("Measured ticks:     %d\n", demo->measured_ticks);
        printf("Measured time:      %.3f\n", demo->measured_ticks * demo->tick_interval);
        printf(RESET);
    }
}

static void demo_gen_tas_script(const Demo* demo, FILE* fp) {
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

int main(int argc, char* argv[]) {
    char* input_file = NULL;
    char* output_file = NULL;

    int mode = QUICK_MODE;
    int parse_level = -1;
    bool debug_mode = false;

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printf("Usage: sdp [-d|-t] [options] <demo> \n");
            printf("Options:\n");
            help("-h, --help", "Display this information.");
            help("-v, --version", "Display parser version information.");
            help("-d, --dump", "Create a text representation of parsed data in the demo.");
            help("-t, --tas", "Create a afterframes TAS script of the demo to be run with the spt plugin.");
            help("-o <file>", "Place the output into <file>.");
            help("-P[level]", "Set the parsing level for dump mode. (default level = 3)");
            help("  -P0", "Parse the header.");
            help("  -P1, -P", "Parse messages");
            help("  -P2", "Parse Net/Svc messages.");
            help("  -P3", "Parse DataTables.");
            help("--debug", "Print debug information.");
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            printf("SourceDemoParser v" VERSION " by evanlin96069.\n");
            return 0;
        }
        else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--dump") == 0) {
            mode = DUMP_MODE;
        }
        else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--tas") == 0) {
            mode = TAS_MODE;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
            else {
                error("Missing filename after \"-o\".\n");
                return 1;
            }
        }
        else if (strncmp(argv[i], "-P", 2) == 0) {
            if (strlen(argv[i]) == 2) {
                parse_level = 1;
            }
            else {
                int8_t level = argv[i][2] - '0';
                if (level < 0 || level > 9) {
                    error("Argument to \"-P\" should be a non-negative integer.\n");
                    return 1;
                }
                parse_level = (uint8_t)level;
            }
        }
        else if (strcmp(argv[i], "--debug") == 0) {
            debug_mode = true;
        }
        else if (!input_file) {
            input_file = argv[i];
        }
        else if (argv[i][0] == '-') {
            error("Unrecognized command line option \"%s\".\n", argv[i]);
            return 1;
        }
        else {
            error("Too many input files.\n");
            return 1;
        }
    }
    if (!input_file) {
        error("No input file.\n");
        return 1;
    }
    if (mode != QUICK_MODE) {
        info("Parsing demo...\n");
        if (!output_file) {
            if (mode == DUMP_MODE) {
                output_file = "out.txt";
            }
            else if (mode == TAS_MODE) {
                output_file = "tas.cfg";
            }
        }
    }

    Demo* demo = new_demo(input_file);

    int result = demo_parse(demo, parse_level, debug_mode);
    if (result == MEASURED_ERROR) {
        demo_free(demo);
        error("Error while parsing demo.\n");
        return 1;
    }
    if (demo->game == GAME_UNKNOWN) {
        warning("Failed to detect game type. Demo might not parsed correctly.\n");
    }
    else {
        info("Game: %s\n", game_names[demo->game]);
    }
    if (demo->tick_interval == 0.0f) {
        warning("Cannot find tick interval, use default value 0.015.\n");
        demo->tick_interval = 0.015f;
    }
    if (mode == QUICK_MODE) {
        print_quick_mode(demo, result);
        getchar();
    }
    else if (mode == DUMP_MODE) {
        info("Dumping verbose output...\n");
        FILE* output = fopen(output_file, "w");
        if (!output) {
            error("Cannot create output file %s.\n", output_file);
            demo_free(demo);
            return 1;
        }
        demo_verbose(demo, output);
        fclose(output);
        info("Text file %s created.\n", output_file);
    }
    else if (mode == TAS_MODE) {
        if (demo->parse_level < 1) {
            error("TAS mode require parse_level >= 1.\n");
            demo_free(demo);
            return 1;
        }
        info("Creating TAS script...\n");
        FILE* output = fopen(output_file, "w");
        if (!output) {
            error("Cannot create output file %s.\n", output_file);
            demo_free(demo);
            return 1;
        }
        demo_gen_tas_script(demo, output);
        fclose(output);
        info("TAS script %s created.\n", output_file);
    }

    demo_free(demo);
    return 0;
}
