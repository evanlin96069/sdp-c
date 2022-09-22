#include "indent_writer.h"
#include "print.h"

static inline void _write_indent() {
    if (g_writer.indent < 0) {
        error("g_writer: negative indent.\n");
        return;
    }
    for (int i = 0; i < g_writer.indent; i++) {
        fputc('\t', g_writer.fp);
    }
}

void write_int(const char* name, int val) {
    _write_indent();
    fprintf(g_writer.fp, "%s: %d\n", name, val);
}

void write_size(const char* name, size_t val) {
    _write_indent();
    fprintf(g_writer.fp, "%s: %zd\n", name, val);
}

void write_bool(const char* name, bool val) {
    _write_indent();
    fprintf(g_writer.fp, "%s: %s\n", name, val ? "true" : "false");
}

void write_float(const char* name, float val) {
    _write_indent();
    fprintf(g_writer.fp, "%s: %.3f\n", name, val);
}

void write_string(const char* name, const char* str) {
    _write_indent();
    fprintf(g_writer.fp, "%s: %s\n", name, str);
}

void write_vec3(const char* name, const float vec[3]) {
    _write_indent();
    fprintf(g_writer.fp, "%s: (%.3f, %.3f, %.3f)\n", name, vec[0], vec[1], vec[2]);
}

void write_nl() {
    fputc('\n', g_writer.fp);
}

void write_line(const char* fmt, ...) {
    _write_indent();
    va_list arg;

    va_start(arg, fmt);
    vfprintf(g_writer.fp, fmt, arg);
    va_end(arg);
}

IndentWriter g_writer = { NULL, 0 };
