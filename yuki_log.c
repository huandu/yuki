#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <pthread.h>

#include "libconfig.h"
#include "yuki.h"

#define YLOG_CONFIG_PATH_LOG_DIR         YUKI_CONFIG_SECTION_YLOG "/log_dir"
#define YLOG_CONFIG_PATH_LOG_FILE        YUKI_CONFIG_SECTION_YLOG "/log_file"
#define YLOG_CONFIG_PATH_MAX_LEVEL       YUKI_CONFIG_SECTION_YLOG "/max_level"
#define YLOG_CONFIG_PATH_MAX_LINE_LENGTH YUKI_CONFIG_SECTION_YLOG "/max_line_length"

#define YUKI_CONFIG_SECTION_YSPECIAL     YUKI_CONFIG_SECTION_YLOG "/special"
#define YLOG_CONFIG_LOG_SPECIAL_LEVEL    "level"
#define YLOG_CONFIG_LOG_SPECIAL_FILE     "log_file"

#define YLOG_MAX_PATH_LENGTH             2048

static yint32_t     g_ylog_max_level = YLOG_LEVEL_MAX;
static ybool_t      g_ylog_inited = yfalse;
static FILE *       g_ylog_file = NULL;
static yint32_t     g_ylog_max_log_line_length = YLOG_MAX_LINE_LENGTH;
static char         g_ylog_real_file[YLOG_MAX_PATH_LENGTH];

static char         g_ylog_dir[YLOG_MAX_PATH_LENGTH];
// TODO: implement different log files for different level
static FILE *       g_ylog_files[YLOG_LEVEL_MAX] = {NULL};
static char         g_ylog_real_files[YLOG_LEVEL_MAX][YLOG_MAX_PATH_LENGTH];
ysize_t             g_ylog_max_type;

static pthread_key_t g_ylog_thread_key;

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

    _YTABLE_CONFIG_STRING(config, YLOG_CONFIG_PATH_LOG_DIR, log_dir);
    _YTABLE_CONFIG_STRING(config, YLOG_CONFIG_PATH_LOG_FILE, log_file);

    struct stat stat_buf;

    if (stat(log_dir, &stat_buf)) {
        if (ENOENT != errno) {
            YUKI_LOG_FATAL("cannot lstat dir '%s'. [errno: %d] [err: %m]", log_dir, errno);
            return yfalse;
        }

        if (mkdir(log_dir, 0751)) {
            YUKI_LOG_FATAL("cannot create dir '%s'. [errno: %d] [err: %m]", log_dir, errno);
            return yfalse;
        }
    } else {
        if (!S_ISDIR(stat_buf.st_mode)) {
            YUKI_LOG_FATAL("log dir '%s' exists but it's not a dir", log_dir);
            return yfalse;
        }
    }

    ysize_t ylog_dir_len = strlen(log_dir);
    ysize_t ylog_file_len = strlen(log_file);
    ysize_t ylog_path_size = ylog_dir_len + ylog_file_len + 1;
    if (ylog_path_size > YLOG_MAX_PATH_LENGTH) {
        YUKI_LOG_FATAL("log dir '%s' is so long", log_dir);
        return yfalse;
    }
    snprintf(g_ylog_real_file, YLOG_MAX_PATH_LENGTH, "%s/%s", log_dir, log_file);
    g_ylog_real_file[ylog_path_size] = 0;

    snprintf(g_ylog_dir,YLOG_MAX_PATH_LENGTH,"%s",log_dir);
    g_ylog_dir[ylog_dir_len] = 0;
    g_ylog_file = fopen(g_ylog_real_file, "a");
    if (!g_ylog_file) {
        YUKI_LOG_FATAL("cannot open log path '%s' for write", g_ylog_real_file);
        return yfalse;
    }

    g_ylog_max_log_line_length = YLOG_MAX_LINE_LENGTH;

    _YTABLE_CONFIG_INT_OPTIONAL(config, YLOG_CONFIG_PATH_MAX_LINE_LENGTH, g_ylog_max_log_line_length, YLOG_MAX_LINE_LENGTH);
    _YTABLE_CONFIG_INT_OPTIONAL(config, YLOG_CONFIG_PATH_MAX_LEVEL,       g_ylog_max_level,           YLOG_LEVEL_MAX);

    // read special settings for special levels
    memset(g_ylog_real_files, 0, YLOG_LEVEL_MAX * YLOG_MAX_PATH_LENGTH);
    ysize_t i;
    yint32_t level;
    config_setting_t* ylog_setting=config_lookup(config, YUKI_CONFIG_SECTION_YSPECIAL);
    if (ylog_setting) {
        if (CONFIG_TYPE_LIST == config_setting_type(ylog_setting)) {
            g_ylog_max_type = config_setting_length(ylog_setting);
            for (i = 0; i < g_ylog_max_type; i++) {
                config_setting_t* ylog_set = config_setting_get_elem(ylog_setting, i);

                _YTABLE_CONFIG_SETTING_INT(ylog_set, YLOG_CONFIG_LOG_SPECIAL_LEVEL, level);
                _YTABLE_CONFIG_SETTING_STRING(ylog_set, YLOG_CONFIG_LOG_SPECIAL_FILE, log_file);

                if (level < 0 || level > YLOG_LEVEL_MAX) {
                    YUKI_LOG_FATAL("get log level '%d' out of range", level);
                    return yfalse;
                }
                ylog_path_size = strlen(log_dir) + strlen(log_file) + 1;
                if (ylog_path_size > YLOG_MAX_PATH_LENGTH) {
                    YUKI_LOG_FATAL("log dir '%s' is so long", log_dir);
                    return yfalse;
                }
                snprintf(g_ylog_real_files[level], YLOG_MAX_PATH_LENGTH, "%s/%s", log_dir, log_file);
                g_ylog_files[level] = fopen(g_ylog_real_files[level], "a");
                if (!g_ylog_file) {
                    YUKI_LOG_FATAL("cannot open log path '%s' for write", g_ylog_real_files[level]);
                    return yfalse;
                }
            }
        }
    }

    int error = pthread_key_create(&g_ylog_thread_key, NULL);

    if (error) {
        YUKI_LOG_FATAL("cannot create thread key for ylog. [err: %d]", error);
        return yfalse;
    }

    g_ylog_inited = ytrue;

    return ytrue;
}

void _ylog_clean_up()
{
    // do nothing
}

void _ylog_shutdown()
{

    ysize_t level;
    for (level = 0; level < YLOG_LEVEL_MAX; level++) {
        if (g_ylog_files[level] != NULL) {
            ylog_flush(level);
            fclose(g_ylog_files[level]);
            g_ylog_files[level] = NULL;
        }
    }

    if (g_ylog_file) {
        fclose(g_ylog_file);
        g_ylog_file = NULL;
    }

    int error = pthread_key_delete(g_ylog_thread_key);
    if (error)
    {
        YUKI_LOG_FATAL("cannot clean g_ylog_thread_key. [err: %d]", error);
    }

    g_ylog_inited = yfalse;
}

void _ylog_write(ylog_level_t level, yint32_t logid, const char * log_header, const char * pattern, ...)
{
    FILE * log_file = stderr;

    if (level <= g_ylog_max_level) {
        char buf[g_ylog_max_log_line_length + 2];
        ysize_t size = g_ylog_max_log_line_length;
        ysize_t offset = 0;
        time_t t = time(NULL);
        offset += strftime(buf + offset, size - offset, "%Y-%m-%d %H:%M:%S ", localtime(&t));
        offset += snprintf(buf + offset, size - offset, "%s ", log_header);
        if (level == YLOG_LEVEL_DEBUG)              
            offset += snprintf(buf + offset, size - offset, "[logid:%d] ", logid);

        va_list args;
        va_start(args, pattern);
        offset += vsnprintf(buf + offset, size - offset, pattern, args);
        va_end(args);

        buf[offset++] = '\n';
        buf[offset++] = '\0';

        if (ylog_inited()) {
            log_file = g_ylog_file;
            if (g_ylog_files[level] != NULL) {
                log_file = g_ylog_files[level];
            }
        }

        fputs(buf, log_file);
        ylog_flush(level);
    }
}

void ylog_flush(ylog_level_t level)
{
    if (ylog_inited()) {
        if (g_ylog_files[level] != NULL) {
            fflush(g_ylog_files[level]);
        }else{
            fflush(g_ylog_file);
        }
    }
}

void ylog_rotate()
{
    YUKI_LOG_DEBUG("ylog_rotate");
    if (ylog_inited()) {
        struct stat stat_buf;
        if (stat(g_ylog_dir, &stat_buf)) {
            if (ENOENT != errno) {
                YUKI_LOG_FATAL("cannot lstat dir '%s'. [errno: %d] [err: %m]", g_ylog_dir, errno);
                return;
            }

            if (mkdir(g_ylog_dir, 0751)) {
                YUKI_LOG_FATAL("cannot create dir '%s'. [errno: %d] [err: %m]", g_ylog_dir, errno);
                return;
            }
        } else {
            if (!S_ISDIR(stat_buf.st_mode)) {
                YUKI_LOG_FATAL("log dir '%s' exists but it's not a dir", g_ylog_dir);
                return;
            }
        }
        FILE * log_file=NULL;
        FILE * old_file=NULL;

        if (g_ylog_file) {
            log_file = fopen(g_ylog_real_file, "a");
            if (!log_file) {
                YUKI_LOG_FATAL("cannot rotate log file. [path: %s]", g_ylog_real_file);
                return;
            }
            YUKI_LOG_DEBUG("log_file=%d g_ylog_path=%s g_ylog_file=%d",log_file,g_ylog_real_file,g_ylog_file);
            old_file = g_ylog_file;
            g_ylog_file = log_file;
            fclose(old_file);
        }
        ysize_t level;

        for (level = 0; level < YLOG_LEVEL_MAX; level++) {
            if (g_ylog_files[level] != NULL) {
                log_file = fopen(g_ylog_real_files[level], "a");
                YUKI_LOG_DEBUG("level=%d log_file=%d g_ylog_path=%s g_ylog_file=%d",level,log_file,g_ylog_real_files[level],g_ylog_files[level]);
                if (!log_file) {
                    YUKI_LOG_FATAL("cannot rotate log file. [path: %s]", g_ylog_real_files[level]);
                    return;
                }
                old_file = g_ylog_files[level];
                g_ylog_files[level] = log_file;
                fclose(old_file);
            }
        }
    }
}


yint32_t ylog_get_pthread_key()
{
    intptr_t value = (intptr_t)pthread_getspecific(g_ylog_thread_key);
    if (value == 0)
    {
        ylog_set_pthread_key();
        value = (intptr_t)pthread_getspecific(g_ylog_thread_key);
    }
    return value;
}

yint32_t ylog_set_pthread_key()
{
    intptr_t value=(intptr_t)rand();
    pthread_setspecific(g_ylog_thread_key,(void*)value);
    return value;
}


