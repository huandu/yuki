#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "yuki.h"

static yint32_t g_ylog_max_level = YLOG_LEVEL_MAX;
static ybool_t g_yuki_log_inited = yfalse;
static FILE * g_yuki_log_file = NULL;

static inline ybool_t yuki_log_inited()
{
    return g_yuki_log_inited;
}

ybool_t _yuki_log_init()
{
    if (yuki_log_inited()) {
        return ytrue;
    }

    g_yuki_log_file = stderr;
    g_yuki_log_inited = ytrue;
    return ytrue;
}

void _yuki_log_clean_up()
{
    // do nothing
}

void _yuki_log_shutdown()
{
    yuki_log_flush();
    g_yuki_log_inited = yfalse;
}

void _yuki_log_write(ylog_level_t level, const char * pattern, ...)
{
    FILE * log_file = stderr;

    if (yuki_log_inited()) {
        log_file = g_yuki_log_file;
    }

    if (level <= g_ylog_max_level) {
        char buf[64] = "\0";
        time_t t = time(NULL);
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", localtime(&t));
        fprintf(log_file, buf);

        va_list args;
        va_start(args, pattern);
        vfprintf(log_file, pattern, args);
        va_end(args);

        fputc('\n', log_file);
    }
}

void yuki_log_flush()
{
    if (yuki_log_inited()) {
        fflush(g_yuki_log_file);
    }
}
