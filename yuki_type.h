#ifndef _YUKI_TYPE_H_
#define _YUKI_TYPE_H_

#ifdef YUKI_CONFIG_C99_ENABLED
# include <inttypes.h>
#else
# include <stdint.h>
# define PRId32 ((sizeof(void *) == 4)? "ld": "d")
# define PRId64 ((sizeof(void *) == 4)? "lld": "ld")
# define PRIu32 ((sizeof(void *) == 4)? "lu": "u")
# define PRIu64 ((sizeof(void *) == 4)? "llu": "lu")
#endif

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YUKI_MAX_INT8_VALUE   127
#define YUKI_MIN_INT8_VALUE   (-128)

#define YUKI_MAX_UINT8_VALUE  255
#define YUKI_MIN_UINT8_VALUE  0

#define YUKI_MAX_INT16_VALUE  32767
#define YUKI_MIN_INT16_VALUE  (-32768)

#define YUKI_MAX_UINT16_VALUE 65535
#define YUKI_MIN_UINT16_VALUE 0

#define YUKI_MAX_INT32_VALUE  2147483647
#define YUKI_MIN_INT32_VALUE  (-YUKI_MAX_INT32_VALUE - 1)

#define YUKI_MAX_UINT32_VALUE 4294967295U
#define YUKI_MIN_UINT32_VALUE 0

#define YUKI_MAX_INT64_VALUE  9223372036854775807LL
#define YUKI_MIN_INT64_VALUE  (-YUKI_MAX_INT64_VALUE - 1LL)

#define YUKI_MAX_UINT64_VALUE 18446744073709551615ULL
#define YUKI_MIN_UINT64_VALUE 0

typedef enum _YBOOL_VALUE {
    yfalse = 0,
    ytrue,
} YBOOL_VALUE;

typedef enum _YVAR_TYPE {
    YVAR_TYPE_UNDEFINED = 0,
    YVAR_TYPE_BOOL,
    YVAR_TYPE_INT8,
    YVAR_TYPE_UINT8,
    YVAR_TYPE_INT16,
    YVAR_TYPE_UINT16,
    YVAR_TYPE_INT32,
    YVAR_TYPE_UINT32,
    YVAR_TYPE_INT64,
    YVAR_TYPE_UINT64,
    YVAR_TYPE_CSTR,
    YVAR_TYPE_STR,
    YVAR_TYPE_ARRAY,
    YVAR_TYPE_LIST,
    YVAR_TYPE_MAP,
} YVAR_TYPE;

/**
 * yuki var options. options can be combined by '|'.
 */
typedef enum _YVAR_OPTIONS {
    YVAR_OPTION_DEFAULT = 0,
    YVAR_OPTION_READONLY = 0x1, /**< yvar_assign() cannot modify readonly var */
    YVAR_OPTION_HOLD_RESOURCE = 0x2, /**< need to free memory */
    YVAR_OPTION_SORTED = 0x4, /**< array is sorted */
} YVAR_OPTIONS;

typedef int8_t ybool_t;
typedef int8_t yint8_t;
typedef uint8_t yuint8_t;
typedef int16_t yint16_t;
typedef uint16_t yuint16_t;
typedef int32_t yint32_t;
typedef uint32_t yuint32_t;
typedef int64_t yint64_t;
typedef uint64_t yuint64_t;
typedef size_t ysize_t;
typedef yuint16_t yvar_option_t;

// forward declaration
struct _yvar_t;
struct _ylist_node_t;

typedef struct _ycstr_t {
    ysize_t size;
    const char * str;
} ycstr_t;

typedef struct _ystr_t {
    ysize_t size;
    char * str;
} ystr_t;

typedef struct _yarray_t {
    ysize_t size;
    struct _yvar_t * yvars;
} yarray_t;

typedef struct _ylist_t {
    struct _ylist_node_t * head;
    struct _ylist_node_t * tail;
} ylist_t;

/**
 * a very simple & stupid 'map'.
 * @note
 * it's not based on tree. it's array.
 */
typedef struct _ymap_t {
    struct _yvar_t * keys;
    struct _yvar_t * values;
} ymap_t;

typedef struct _yvar_t {
    yuint8_t type;
    yuint8_t version;
    yvar_option_t options;

    union {
        yint8_t yundefined_data; // should be always 0
        ybool_t ybool_data;
        yint8_t yint8_data;
        yuint8_t yuint8_data;
        yint16_t yint16_data;
        yuint16_t yuint16_data;
        yint32_t yint32_data;
        yuint32_t yuint32_data;
        yint64_t yint64_data;
        yuint64_t yuint64_data;
        ycstr_t ycstr_data;
        ystr_t ystr_data;
        yarray_t yarray_data;
        ylist_t ylist_data;
        ymap_t ymap_data;
    } data;
} yvar_t;

typedef struct _ylist_node_t {
    struct _ylist_node_t * prev;
    struct _ylist_node_t * next;
    yvar_t yvar;
} ylist_node_t;

typedef struct _ytable_t {
    yvar_t * condition;
    yvar_t * params;
    yvar_t * hash_key;
    yvar_t * sql;
    ysize_t table_index;
} ytable_t;

typedef struct _ytable_config_t {
    char * table_name;
    char * hash_method;
    yvar_t * params;
} ytable_config_t;

typedef struct _ybuffer_t {
    ysize_t size;
    ysize_t offset;
    struct _ybuffer_t * next;
    char buffer[];
} ybuffer_t;

/**
 * cookie for global buffer.
 */
typedef struct _ybuffer_cookie_t {
    ybuffer_t * prev;
    yuint64_t padding;
} ybuffer_cookie_t;

#ifdef __cplusplus
}
#endif

#endif
