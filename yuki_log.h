#ifndef _YUKI_LOG_H_
#define _YUKI_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _YLOG_FORMAT(prefix, file, line) _YLOG_FORMAT_REAL(prefix, file, line)
#define _YLOG_FORMAT_REAL(prefix, file, line) "["prefix"] ["file":"#line"]"

#define YUKI_LOG_CRITICAL(...) _ylog_write(YLOG_LEVEL_CRITICAL, _YLOG_FORMAT("CRITICAL", __FILE__, __LINE__), __VA_ARGS__)
#define YUKI_LOG_FATAL(...)    _ylog_write(YLOG_LEVEL_FATAL,    _YLOG_FORMAT("FATAL", __FILE__, __LINE__), __VA_ARGS__)
#define YUKI_LOG_WARNING(...)  _ylog_write(YLOG_LEVEL_WARNING,  _YLOG_FORMAT("WARNING", __FILE__, __LINE__), __VA_ARGS__)
#define YUKI_LOG_NOTICE(...)   _ylog_write(YLOG_LEVEL_NOTICE    _YLOG_FORMAT("NOTICE", __FILE__, __LINE__), __VA_ARGS__)
#define YUKI_LOG_TRACE(...)    _ylog_write(YLOG_LEVEL_TRACE,    _YLOG_FORMAT("TRACE", __FILE__, __LINE__), __VA_ARGS__)
#define YUKI_LOG_DEBUG(...)    _ylog_write(YLOG_LEVEL_DEBUG,    _YLOG_FORMAT("DEBUG", __FILE__, __LINE__), __VA_ARGS__)

#define YLOG_MAX_LINE_LENGTH 1024

typedef enum _ylog_level_t {
    YLOG_LEVEL_CRITICAL = 0,
    YLOG_LEVEL_FATAL = 1,
    YLOG_LEVEL_WARNING = 4,
    YLOG_LEVEL_NOTICE = 8,
    YLOG_LEVEL_TRACE = 16,
    YLOG_LEVEL_DEBUG = 32,
    YLOG_LEVEL_MAX,
} ylog_level_t;

void _ylog_write(ylog_level_t level, const char * log_header, const char * pattern, ...);
void ylog_flush();

#ifdef __cplusplus
}
#endif

#endif
