#ifndef _YUKI_BUFFER_H_
#define _YUKI_BUFFER_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _YBUFFER_ALLOC_ALIGN 8

#define YBUFFER_COOKIE_PADDING ((yuint64_t)0xF3C18304DC21A5B7ULL)

#define ybuffer_smart_alloc(b, t) (t*)ybuffer_alloc((b), sizeof(t))
#define ybuffer_round_up(s) (((s) + _YBUFFER_ALLOC_ALIGN - 1) & ~(_YBUFFER_ALLOC_ALIGN - 1))

ybuffer_t * ybuffer_create(ysize_t size);
ybuffer_t * ybuffer_create_global(ysize_t size);
void * ybuffer_alloc(ybuffer_t * buffer, ysize_t size);
ysize_t ybuffer_available_size(const ybuffer_t * buffer);
ybool_t ybuffer_destroy_global(ybuffer_t * buffer);
ybool_t ybuffer_destroy_global_pointer(void * pointer);

#ifdef __cplusplus
}
#endif

#endif
