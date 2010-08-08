#include <stdio.h>
#include <time.h>
#include <stdarg.h>

#include "yuki.h"

static yint32_t g_ylog_max_level = YLOG_LEVEL_DEBUG;
static ybool_t g_yuki_log_inited = yfalse;
static FILE * g_yuki_log_file = NULL;

static inline ybool_t _yuki_log_inited()
{
    return g_yuki_log_inited;
}

ybool_t yuki_log_init()
{
    g_yuki_log_file = stderr;
    g_yuki_log_inited = ytrue;
    return ytrue;
}

ybool_t _yuki_log_write(ylog_level_t level, const char * pattern, ...)
{
    if (!_yuki_log_inited()) {
        return yfalse;
    }

    if (level <= g_ylog_max_level) {
        char buf[64] = "\0";
        time_t t = time(NULL);
        strftime(buf, sizeof(buf), "\n%Y-%m-%d %H:%I:%S ", localtime(&t));
        fprintf(g_yuki_log_file, buf);

        va_list args;
        va_start(args, pattern);
        vfprintf(g_yuki_log_file, pattern, args);
        va_end(args);
    }

    return ytrue;
}
