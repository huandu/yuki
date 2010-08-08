#ifndef _YUKI_MEMORY_H_
#define _YUKI_MEMORY_H_

#ifdef __cplusplus
extern "C" {
#endif

ybool_t yuki_init(const char * config_filename);
ybool_t yuki_clean_up();
ybool_t yuki_shutdown();
void * yuki_alloc(ysize_t size);

#ifdef __cplusplus
}
#endif

#endif
