#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>
#include "ansi-style.h"

#ifdef _DEBUG
#define debug(...) fprintf(stderr, "[DEBUG] " __VA_ARGS__)
#else
#define debug(...) do { demo_info.debug_mode ? fprintf(stderr, "[DEBUG] " __VA_ARGS__) : 0; } while(0)
#endif

#define error(...) fprintf(stderr, "["SET_FG(RED) "ERROR" RESET "] " __VA_ARGS__)
#define warning( ...) fprintf(stderr, "[" SET_FG(YEL) "WARNING" RESET "] " __VA_ARGS__)
#define info(...) fprintf(stdout, "[INFO] " __VA_ARGS__)

#endif