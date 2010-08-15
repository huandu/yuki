#ifndef _YUKI_LOG_H_
#define _YUKI_LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _YUKI_LOG_FORMAT(prefix, file, line) _YUKI_LOG_FORMAT_REAL(prefix, file, line)
#define _YUKI_LOG_FORMAT_REAL(prefix, file, line) "["prefix"] ["file":"#line"] "

#define YUKI_LOG_CRITICAL(...) _yuki_log_write(YLOG_LEVEL_CRITICAL, _YUKI_LOG_FORMAT("CRITICAL", __FILE__, __LINE__) __VA_ARGS__)
#define YUKI_LOG_FATAL(...)    _yuki_log_write(YLOG_LEVEL_FATAL,    _YUKI_LOG_FORMAT("FATAL", __FILE__, __LINE__) __VA_ARGS__)
#define YUKI_LOG_WARNING(...)  _yuki_log_write(YLOG_LEVEL_WARNING,  _YUKI_LOG_FORMAT("WARNING", __FILE__, __LINE__) __VA_ARGS__)
#define YUKI_LOG_NOTICE(...)   _yuki_log_write(YLOG_LEVEL_NOTICE    _YUKI_LOG_FORMAT("NOTICE", __FILE__, __LINE__) __VA_ARGS__)
#define YUKI_LOG_TRACE(...)    _yuki_log_write(YLOG_LEVEL_TRACE,    _YUKI_LOG_FORMAT("TRACE", __FILE__, __LINE__) __VA_ARGS__)
#define YUKI_LOG_DEBUG(...)    _yuki_log_write(YLOG_LEVEL_DEBUG,    _YUKI_LOG_FORMAT("DEBUG", __FILE__, __LINE__) __VA_ARGS__)

typedef enum _ylog_level_t {
    YLOG_LEVEL_CRITICAL = 0,
    YLOG_LEVEL_FATAL = 1,
    YLOG_LEVEL_WARNING = 4,
    YLOG_LEVEL_NOTICE = 8,
    YLOG_LEVEL_TRACE = 16,
    YLOG_LEVEL_DEBUG = 32,
    YLOG_LEVEL_MAX,
} ylog_level_t;

void _yuki_log_write(ylog_level_t level, const char * format, ...);
void yuki_log_flush();

#ifdef __cplusplus
}
#endif

#endif
