#ifndef _YUKI_INIT_H_
#define _YUKI_INIT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YUKI_COMPONENT_DECLARE(c) \
    extern ybool_t _##c##_init(); \
    extern void _##c##_clean_up(); \
    extern void _##c##_shutdown();

#define YUKI_COMPONENT_BEGIN() \
    static yuki_component_t g_components[] = {

#define YUKI_COMPONENT_REGISTER(c) \
        {#c, &_##c##_init, &_##c##_clean_up, &_##c##_shutdown},

#define YUKI_COMPONENT_END() \
    };

typedef ybool_t (*yuki_init_callback)();
typedef void (*yuki_clean_up_callback)();
typedef void (*yuki_shutdown_callback)();

typedef struct _yuki_component_t {
    const char * name;
    yuki_init_callback init;
    yuki_clean_up_callback clean_up;
    yuki_shutdown_callback shutdown;
} yuki_component_t;

/**
 * init necessary global variables/component.
 */
ybool_t yuki_init();

/**
 * clean up thread data.
 * as yuki lib uses a managed memory pool and NEVER auto free memory,
 * it's highly recommended to do clean up action regularly.
 */
void yuki_clean_up();

/**
 * fully shutdown yuki lib.
 */
void yuki_shutdown();

#ifdef __cplusplus
}
#endif

#endif
