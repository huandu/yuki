#include "yuki.h"

YUKI_COMPONENT_DECLARE(yuki_log)
YUKI_COMPONENT_DECLARE(ybuffer)
YUKI_COMPONENT_DECLARE(ytable)

YUKI_COMPONENT_BEGIN()
    YUKI_COMPONENT_REGISTER(yuki_log)
    YUKI_COMPONENT_REGISTER(ybuffer)
    YUKI_COMPONENT_REGISTER(ytable)
YUKI_COMPONENT_END()

static ybool_t g_yuki_inited = yfalse;

yuki_component_t * yuki_component_get()
{
    return g_components;
}

ybool_t yuki_init(const char * config_filename)
{
    if (!g_yuki_inited) {
        YUKI_LOG_TRACE("start to init...");

        for (yuki_component_t * component = yuki_component_get();
            component->name; component++) {
            YUKI_LOG_TRACE("try to init %s", component->name);

            if (!component->init()) {
                YUKI_LOG_FATAL("cannot init %s", component->name);
                yuki_shutdown();
                return yfalse;
            }

            YUKI_LOG_TRACE("%s is init-ed", component->name);
        }

        g_yuki_inited = ytrue;
        YUKI_LOG_TRACE("init completed");
    }

    return ytrue;
}

void yuki_clean_up()
{
    for (yuki_component_t * component = yuki_component_get();
        component->name; component++) {
        component->clean_up();
    }
}

void yuki_shutdown()
{
    for (yuki_component_t * component = yuki_component_get();
        component->name; component++) {
        component->shutdown();
    }

    g_yuki_inited = yfalse;
}
