#ifndef PRINT_H
#define PRINT_H

#include <stdio.h>

#ifdef _DEBUG
#define debug(...) fprintf(stderr, "[DEBUG] " __VA_ARGS__)
#else
#define debug(...) do{}while(0)
#endif

#define error(...) fprintf(stderr, "[ERROR] " __VA_ARGS__)
#define warning( ...) fprintf(stderr, "[WARNING] " __VA_ARGS__)
#define info(...) fprintf(stdout, "[INFO] " __VA_ARGS__)

#endif