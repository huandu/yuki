#include <errno.h>
#include <stdlib.h>
#include <pthread.h>

#include "yuki.h"

static pthread_key_t g_ybuffer_thread_key;
static ybool_t g_ybuffer_inited = yfalse;

static void _ybuffer_thread_clean_up(void * head)
{
    if (!head) {
        YUKI_LOG_DEBUG("buffer chain is empty");
        return;
    }

    ybuffer_t * buffer = (ybuffer_t*)head;
    ybuffer_t * next = NULL;

    while (buffer) {
        next = buffer->next;
        free(buffer);
        buffer = next;
    }
}

static inline ybool_t ybuffer_inited()
{
    return g_ybuffer_inited;
}

ybool_t _ybuffer_init()
{
    if (ybuffer_inited()) {
        return ytrue;
    }

    if (pthread_key_create(&g_ybuffer_thread_key, &_ybuffer_thread_clean_up)) {
        YUKI_LOG_FATAL("cannot create thread key for ybuffer. [err: %d]", errno);
        return yfalse;
    }

    g_ybuffer_inited = ytrue;
    return ytrue;
}

void _ybuffer_clean_up()
{
    ybuffer_t * head = (ybuffer_t*)pthread_getspecific(g_ybuffer_thread_key);
    pthread_setspecific(g_ybuffer_thread_key, NULL);
    _ybuffer_thread_clean_up(head);
}

void _ybuffer_shutdown()
{
    g_ybuffer_inited = yfalse;
}

ybuffer_t * ybuffer_create(ysize_t size)
{
    if (!g_ybuffer_inited) {
        YUKI_LOG_FATAL("ybuffer is not init-ed");
        return NULL;
    }

    ysize_t rounded = ybuffer_round_up(size);
    ybuffer_t * ptr = (ybuffer_t*)malloc(sizeof(ybuffer_t) + rounded);

    if (!ptr) {
        YUKI_LOG_FATAL("out of memory. [size: %lu] [actual: %lu]", size, sizeof(ybuffer_t) + rounded);
        return NULL;
    }

    ptr->size = rounded;
    ptr->offset = 0;

    // add memory to free list
    ybuffer_t * head = (ybuffer_t*)pthread_getspecific(g_ybuffer_thread_key);

    if (head) {
        ptr->next = head->next;
        head->next = ptr;
    } else {
        ptr->next = NULL;
        pthread_setspecific(g_ybuffer_thread_key, ptr);
    }

    return ptr;
}

void * ybuffer_alloc(ybuffer_t * buffer, ysize_t size)
{
    if (!buffer) {
        YUKI_LOG_FATAL("invalid buffer pool");
        return NULL;
    }

    ysize_t rounded = ybuffer_round_up(size);

    if (buffer->offset + rounded > buffer->size) {
        YUKI_LOG_FATAL("not enough memory in buffer pool");
        return NULL;
    }

    char * ret = buffer->buffer + buffer->offset;
    buffer->offset += rounded;
    return (void *)ret;
}

ysize_t ybuffer_available_size(const ybuffer_t * buffer)
{
    if (!buffer) {
        YUKI_LOG_FATAL("invalid buffer pool");
        return 0;
    }

    return buffer->size - buffer->offset;
}
