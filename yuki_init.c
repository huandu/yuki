#include "libconfig.h"
#include "yuki.h"

YUKI_COMPONENT_DECLARE(ylog)
YUKI_COMPONENT_DECLARE(ybuffer)
YUKI_COMPONENT_DECLARE(ytable)

YUKI_COMPONENT_BEGIN()
    YUKI_COMPONENT_REGISTER(ylog)
    YUKI_COMPONENT_REGISTER(ybuffer)
    YUKI_COMPONENT_REGISTER(ytable)
YUKI_COMPONENT_END()

static ybool_t g_yuki_inited = yfalse;

static yuki_component_t * _yuki_component_get_begin()
{
    return g_components;
}

static yuki_component_t * _yuki_component_get_end()
{
    return g_components + sizeof(g_components) / sizeof(g_components[0]);
}

static yuki_component_t * _yuki_component_get_rbegin()
{
    return g_components + sizeof(g_components) / sizeof(g_components[0]) - 1;
}

static yuki_component_t * _yuki_component_get_rend()
{
    return g_components - 1;
}

ybool_t yuki_init(const char * filename)
{
    if (!filename) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!g_yuki_inited) {
        YUKI_LOG_TRACE("start to init...");

        config_t config;
        config_init(&config);

        if (CONFIG_TRUE != config_read_file(&config, filename)) {
            YUKI_LOG_FATAL("cannot read config file. [filename: %s]", filename);
            config_destroy(&config);
            return yfalse;
        }

        YUKI_LOG_TRACE("reading config file '%s'...", filename);

        yuki_component_t * first = _yuki_component_get_begin();
        yuki_component_t * last = _yuki_component_get_end();

        for (; first != last; first++) {
            YUKI_LOG_TRACE("try to init %s", first->name);

            if (!first->init(&config)) {
                YUKI_LOG_FATAL("cannot init %s", first->name);
                yuki_shutdown();
                config_destroy(&config);
                return yfalse;
            }

            YUKI_LOG_TRACE("%s is init-ed", first->name);
        }

        g_yuki_inited = ytrue;
        config_destroy(&config);
        YUKI_LOG_TRACE("init completed");
    }

    return ytrue;
}

void yuki_clean_up()
{
    yuki_component_t * first = _yuki_component_get_begin();
    yuki_component_t * last = _yuki_component_get_end();
    
    for (; first != last; first++) {
        first->clean_up();
    }
}

void yuki_shutdown()
{
    yuki_component_t * first = _yuki_component_get_rbegin();
    yuki_component_t * last = _yuki_component_get_rend();
    
    for (; first != last; first--) {
        first->shutdown();
    }

    g_yuki_inited = yfalse;
}
