#include <errno.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "yuki.h"

static pthread_key_t g_ybuffer_thread_key;
static ybool_t g_ybuffer_inited = yfalse;
static ybuffer_t * g_ybuffer_global_chain = NULL;

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

static void _ybuffer_global_clean_up()
{
    if (!g_ybuffer_global_chain) {
        YUKI_LOG_DEBUG("global buffer chain is empty");
        return;
    }

    ybuffer_t * buffer = g_ybuffer_global_chain;
    ybuffer_t * next = NULL;
    ybuffer_cookie_t * cookie = NULL;

    while (buffer) {
        next = buffer->next;

        // destroy padding
        YUKI_ASSERT(buffer->size >= ybuffer_round_up(sizeof(ybuffer_cookie_t)));
        cookie = (ybuffer_cookie_t*)buffer->buffer;
        cookie->padding = 0;

        free(buffer);
        buffer = next;
    }

    g_ybuffer_global_chain = NULL;
}

static inline ybool_t ybuffer_inited()
{
    return g_ybuffer_inited;
}

static void _ybuffer_thread_chain_add(ybuffer_t * buffer)
{
    // add memory to free list
    ybuffer_t * head = (ybuffer_t*)pthread_getspecific(g_ybuffer_thread_key);

    if (head) {
        buffer->next = head->next;
        head->next = buffer;
    } else {
        buffer->next = NULL;
        pthread_setspecific(g_ybuffer_thread_key, buffer);
    }
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
    _ybuffer_global_clean_up();
    g_ybuffer_inited = yfalse;
}

/**
 * create a managed buffer.
 * @note
 * this buffer is available in current thread.
 * do NEVER use it cross thread.
 */
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

    _ybuffer_thread_chain_add(ptr);

    return ptr;
}

/**
 * create a global buffer available in every thread.
 * the memory is always available until
 * ybuffer_destroy_global() or ybuffer_destroy_global_pointer() is called
 * against this buffer.
 * @note
 * global buffer will be auto freed when _ybuffer_shutdown() is called.
 */
ybuffer_t * ybuffer_create_global(ysize_t size)
{
    if (!g_ybuffer_inited) {
        YUKI_LOG_FATAL("ybuffer is not init-ed");
        return NULL;
    }

    ysize_t rounded = ybuffer_round_up(size);
    ysize_t cookie_size = ybuffer_round_up(sizeof(ybuffer_cookie_t));
    ybuffer_t * ptr = (ybuffer_t*)malloc(sizeof(ybuffer_t) + cookie_size + rounded);

    if (!ptr) {
        YUKI_LOG_FATAL("out of memory. [size: %lu] [actual: %lu]", size, sizeof(ybuffer_t) + rounded);
        return NULL;
    }

    // first element in buffer is the pointer points to previous buffer.
    ptr->size = rounded + cookie_size;
    ptr->offset = cookie_size;
    ybuffer_cookie_t * cookie = (ybuffer_cookie_t*)ptr->buffer;
    cookie->padding = YBUFFER_COOKIE_PADDING;

    // add memory to global free list
    if (g_ybuffer_global_chain) {
        ptr->next = g_ybuffer_global_chain->next;

        if (ptr->next) {
            ((ybuffer_cookie_t*)ptr->next->buffer)->prev = ptr;
        }

        g_ybuffer_global_chain->next = ptr;
        cookie->prev = g_ybuffer_global_chain;
    } else {
        ptr->next = NULL;
        cookie->prev = NULL;
        g_ybuffer_global_chain = ptr;
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

/**
 * remove a global buffer from global chain
 * and add it to thread chain.
 */
ybool_t ybuffer_destroy_global(ybuffer_t * buffer)
{
    if (!buffer) {
        YUKI_LOG_FATAL("invalid buffer pool");
        return yfalse;
    }

    ybuffer_cookie_t * cookie = (ybuffer_cookie_t*)buffer->buffer;

    if (YBUFFER_COOKIE_PADDING != cookie->padding) {
        YUKI_LOG_WARNING("try to destroy an invalid global buffer");
        return yfalse;
    }

    ybuffer_t * prev = cookie->prev;
    ybuffer_t * next = buffer->next;
    cookie->padding = 0;

    if (next) {
        ((ybuffer_cookie_t*)next->buffer)->prev = prev;
    }

    if (prev) {
        prev->next = next;
    } else {
        g_ybuffer_global_chain = next;
    }

    _ybuffer_thread_chain_add(buffer);
    return ytrue;
}

/**
 * destroy buffer through the pointer of first element allocated in global buffer.
 * it's typically used by yvar_unpin() to unpin a pinned yvar.
 */
ybool_t ybuffer_destroy_global_pointer(void * pointer)
{
    if (!pointer) {
        YUKI_LOG_FATAL("invalid pointer");
        return yfalse;
    }

    ybuffer_t * buffer = (ybuffer_t*)((char*)pointer - ybuffer_round_up(sizeof(ybuffer_cookie_t)) - sizeof(ybuffer_t));
    return ybuffer_destroy_global(buffer);
}
