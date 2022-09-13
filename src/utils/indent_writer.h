#ifndef INDENT_WRITER_H
#define INDENT_WRITER_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdarg.h>

void write_int(const char* name, int val);
void write_size(const char* name, size_t val);
void write_bool(const char* name, bool val);
void write_float(const char* name, float val);
void write_string(const char* name, const char* str);
void write_vec3(const char* name, const float vec[3]);
void write_nl();
void write_line(const char* fmt, ...);

typedef struct {
    FILE* fp;
    int indent;
} IndentWriter;
extern IndentWriter g_writer;

#endif