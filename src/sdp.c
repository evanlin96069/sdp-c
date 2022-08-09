#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "demo.h"
#include "demo_info.h"

int get_build_number();

enum {
    QUICK_MODE,
    VERBOSE_MODE,
    TAS_MODE
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: sdp [options] <demo>\n");
    }

    int build_number = get_build_number();

    char* input_file = NULL;
    char* output_file = NULL;
    int mode = QUICK_MODE;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: sdp [options] <demo> \n");
            printf("Options:\n");
            printf("--help          Display this information.\n");
            printf("--version       Display version.\n");
            printf("-v              Dump parsable info into a text file.\n");
            printf("-t              Convert demo file to .cfg TAS file.\n");
            printf("-o <file>       Place the output into <file>.\n");
            return 0;
        }
        else if (strcmp(argv[i], "--version") == 0) {
            printf("sdp (build %d) by evanlin96069.\n", build_number);
            return 0;
        }
        else if (strcmp(argv[i], "-v") == 0) {
            mode = VERBOSE_MODE;
        }
        else if (strcmp(argv[i], "-t") == 0) {
            mode = TAS_MODE;
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[++i];
            }
            else {
                fprintf(stderr, "[ERROR] Missing filename after \"-o\"\n");
                return 1;
            }
        }
        else if (!input_file) {
            input_file = argv[i];
        }
        else if (argv[i][0] == '-') {
            fprintf(stderr, "[ERROR] Unrecognized command line option \"%s\"\n", argv[i]);
            return 1;
        }
        else {
            fprintf(stderr, "[ERROR] Too many input files.\n");
            return 1;
        }
    }
    if (!input_file) {
        fprintf(stderr, "[ERROR] No input file.\n");
        return 1;
    }
    if (mode != QUICK_MODE && !output_file) {
        if (mode == VERBOSE_MODE) {
            output_file = "out.txt";
        }
        else if (mode == TAS_MODE) {
            output_file = "tas.cfg";
        }
    }

    Demo* demo = new_demo(input_file);

    if (mode != QUICK_MODE) {
        printf("[INFO] Parsing demo...\n");
    }
    int measured_ticks = demo_parse(demo, mode == QUICK_MODE);
    if (measured_ticks < 0) {
        demo_free(demo);
        fprintf(stderr, "[ERROR] Error while parsing demo.\n");
        return 1;
    }
    if (mode == QUICK_MODE) {
        const DemoHeader* header = &demo->header;
        printf("FileName:           %s\n", demo->file_name);
        printf("DemoFileStamp:      %s\n", header->demo_file_stamp);
        printf("DemoProtocol:       %d\n", header->demo_protocol);
        printf("NetworkProtocol:    %d\n", header->network_protocol);
        printf("ServerName:         %s\n", header->server_name);
        printf("ClientName:         %s\n", header->client_name);
        printf("MapName:            %s\n", header->map_name);
        printf("GameDirectory:      %s\n", header->game_dir);
        printf("PlayBackTime:       %.3f\n", header->play_back_time);
        printf("PlayBackTicks:      %d\n", header->play_back_ticks);
        printf("PlayBackFrames:     %d\n", header->play_back_frames);
        printf("SignOnLength:       %d\n\n", header->sign_on_length);
        printf("Measured ticks:     %d\n", measured_ticks);
        printf("Measured time:      %.3f\n", measured_ticks / demo_info.tickrate);
    }
    else if (mode == VERBOSE_MODE) {
        printf("[INFO] Dumping verbose output...\n");
        FILE* output = fopen(output_file, "w");
        if (!output) {
            fprintf(stderr, "Cannot create output file %s.\n", output_file);
            demo_free(demo);
            return 1;
        }
        demo_verbose(demo, output);
        fclose(output);
        printf("[INFO] Text file %s created.\n", output_file);
    }
    else if (mode == TAS_MODE) {
        printf("[INFO] Creating TAS script...\n");
        FILE* output = fopen(output_file, "w");
        if (!output) {
            fprintf(stderr, "Cannot create output file %s.\n", output_file);
            demo_free(demo);
            return 1;
        }
        demo_gen_tas_script(demo, output);
        fclose(output);
        printf("[INFO] TAS script %s created.\n", output_file);
    }

    demo_free(demo);
    return 0;
}

int get_build_number() {
    char* date = __DATE__;
    char* month[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
    char day[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    int m = 0;
    int d = 0;
    int y = 0;

    for (m = 0; m < 11; m++) {
        if (strncmp(&date[0], month[m], 3) == 0)
            break;
        d += day[m];
    }
    d += atoi(&date[4]) - 1;
    y = atoi(&date[7]) - 1900;

    int build = d + (int)((y - 1) * 365.25);
    if (((y % 4) == 0) && m > 1) {
        build += 1;
    }
    build -= 43720;  // Sep 13 2020
    return build;
}
