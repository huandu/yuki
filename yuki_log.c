#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>

#include "libconfig.h"
#include "yuki.h"

#define YLOG_CONFIG_PATH_LOG_DIR         YUKI_CONFIG_SECTION_YLOG "/log_dir"
#define YLOG_CONFIG_PATH_LOG_FILE        YUKI_CONFIG_SECTION_YLOG "/log_file"
#define YLOG_CONFIG_PATH_MAX_LEVEL       YUKI_CONFIG_SECTION_YLOG "/max_level"
#define YLOG_CONFIG_PATH_MAX_LINE_LENGTH YUKI_CONFIG_SECTION_YLOG "/max_line_length"

static yint32_t g_ylog_max_level = YLOG_LEVEL_MAX;
static ybool_t g_ylog_inited = yfalse;
static FILE * g_ylog_file = NULL;
static yint32_t g_ylog_max_log_line_length = YLOG_MAX_LINE_LENGTH;

static inline ybool_t ylog_inited()
{
    return g_ylog_inited;
}

ybool_t _ylog_init(config_t * config)
{
    if (ylog_inited()) {
        return ytrue;
    }

    const char * log_dir;
    const char * log_file;

    if (CONFIG_TRUE != config_lookup_string(config, YLOG_CONFIG_PATH_LOG_DIR, &log_dir)) {
        YUKI_LOG_FATAL("cannot get '%s' in config file", YLOG_CONFIG_PATH_LOG_DIR);
        return yfalse;
    }

    if (CONFIG_TRUE != config_lookup_string(config, YLOG_CONFIG_PATH_LOG_FILE, &log_file)) {
        YUKI_LOG_FATAL("cannot get '%s' in config file", YLOG_CONFIG_PATH_LOG_FILE);
        return yfalse;
    }

    struct stat stat_buf;

    if (stat(log_dir, &stat_buf)) {
        if (ENOENT != errno) {
            YUKI_LOG_FATAL("cannot lstat dir '%s'. [errno: %d] [err: %m]", log_dir, errno);
            return yfalse;
        }

        if (mkdir(log_dir, 0777)) {
            YUKI_LOG_FATAL("cannot create dir '%s'. [errno: %d] [err: %m]", log_dir, errno);
            return yfalse;
        }
    } else {
        if (!S_ISDIR(stat_buf.st_mode)) {
            YUKI_LOG_FATAL("log dir '%s' exists but it's not a dir", log_dir);
            return yfalse;
        }
    }

    ysize_t log_path_size = strlen(log_dir) + strlen(log_file) + 1;
    char log_path[log_path_size];
    snprintf(log_path, log_path_size, "%s%s", log_dir, log_file);

    g_ylog_file = fopen(log_path, "a");

    if (!g_ylog_file) {
        YUKI_LOG_FATAL("cannot open log path '%s' for write", log_path);
        return yfalse;
    }

    g_ylog_max_log_line_length = YLOG_MAX_LINE_LENGTH;
    g_ylog_inited = ytrue;

    YUKI_LOG_DEBUG("%s is set to %s", YLOG_CONFIG_PATH_LOG_DIR, log_dir);
    YUKI_LOG_DEBUG("%s is set to %s", YLOG_CONFIG_PATH_LOG_FILE, log_file);

    if (CONFIG_TRUE == config_lookup_int(config, YLOG_CONFIG_PATH_MAX_LINE_LENGTH, &g_ylog_max_log_line_length)) {
        YUKI_LOG_DEBUG("%s is set to %d", YLOG_CONFIG_PATH_MAX_LINE_LENGTH, g_ylog_max_log_line_length);
    }

    if (CONFIG_TRUE == config_lookup_int(config, YLOG_CONFIG_PATH_MAX_LEVEL, &g_ylog_max_level)) {
        YUKI_LOG_DEBUG("%s is set to %d", YLOG_CONFIG_PATH_MAX_LEVEL, g_ylog_max_level);
    }

    return ytrue;
}

void _ylog_clean_up()
{
    // do nothing
}

void _ylog_shutdown()
{
    ylog_flush();

    if (g_ylog_file) {
        fclose(g_ylog_file);
        g_ylog_file = NULL;
    }

    g_ylog_inited = yfalse;
}

void _ylog_write(ylog_level_t level, const char * log_header, const char * pattern, ...)
{
    FILE * log_file = stderr;

    if (ylog_inited()) {
        log_file = g_ylog_file;
    }

    if (level <= g_ylog_max_level) {
        char buf[g_ylog_max_log_line_length + 2];
        ysize_t size = g_ylog_max_log_line_length;
        ysize_t offset = 0;
        time_t t = time(NULL);
        offset += strftime(buf + offset, size - offset, "%Y-%m-%d %H:%M:%S ", localtime(&t));
        offset += snprintf(buf + offset, size - offset, "%s ", log_header);

        va_list args;
        va_start(args, pattern);
        offset += vsnprintf(buf + offset, size - offset, pattern, args);
        va_end(args);

        buf[offset++] = '\n';
        buf[offset++] = '\0';
        fputs(buf, log_file);
    }
}

void ylog_flush()
{
    if (ylog_inited()) {
        fflush(g_ylog_file);
    }
}
