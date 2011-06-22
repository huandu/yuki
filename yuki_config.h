#ifndef _YUKI_CONFIG_H_
#define _YUKI_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__STDC_VERSION__))
# if (__STDC_VERSION__ == 199901L)
#  define YUKI_CONFIG_C99_ENABLED
# endif
#endif

#define YUKI_CONFIG_SECTION_YBUFFER "ybuffer"
#define YUKI_CONFIG_SECTION_YTABLE  "ytable"
#define YUKI_CONFIG_SECTION_YLOG    "ylog"



#define _YTABLE_CONFIG_SETTING_STRING(config, path, var) do { \
        if (CONFIG_TRUE != config_setting_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_FATAL("element must have member '%s'", \
                (path)); \
            return yfalse; \
        } \
        YUKI_LOG_DEBUG("'%s' is set to '%s'", (path), (var)); \
    } while (0)

#define _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_setting_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%s'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%s'", \
                (path), (def)); \
        } \
    } while (0)

#define _YTABLE_CONFIG_SETTING_INT(config, path, var) do { \
        if (CONFIG_TRUE != config_setting_lookup_int((config), (path), &(var))) { \
            YUKI_LOG_FATAL("element must have member '%s'", \
                (path)); \
            return yfalse; \
        } \
        YUKI_LOG_DEBUG("'%s' is set to '%d'", (path), (var)); \
    } while (0)

#define _YTABLE_CONFIG_SETTING_INT_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_setting_lookup_int((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%d'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%d'", \
                (path), (def)); \
        } \
    } while (0)


#define _YTABLE_CONFIG_STRING(config, path, var) do { \
        if (CONFIG_TRUE != config_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_FATAL("config must have member '%s'", \
                (path)); \
            return yfalse; \
        } \
        YUKI_LOG_DEBUG("'%s' is set to '%s'", (path), (var)); \
    } while (0)

#define _YTABLE_CONFIG_STRING_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%s'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%s'", \
                (path), (def)); \
        } \
    } while (0)

#define _YTABLE_CONFIG_INT_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_lookup_int((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%d'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%d'", \
                (path), (def)); \
        } \
    } while (0)



#ifdef __cplusplus
}
#endif

#endif
