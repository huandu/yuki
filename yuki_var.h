#ifndef _YUKI_VAR_H_
#define _YUKI_VAR_H_

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#define YUKI_VAR_VERSION 1

#if (defined(__STDC_VERSION__))
#   if (__STDC_VERSION__ == 199901L)
#       define YUKI_CONSTANTS_C99_ENABLED
#   endif
#endif

#define _YVAR_TEMP_VARIABLE(p, s) __yvar_temp##p##s

#define YCSTR(s) {.size = sizeof((s)) - 1, .str = s}

#define _YVAR_INIT(t, tn, ...) {.type = (t), .version = YUKI_VAR_VERSION, .options = YVAR_OPTION_DEFAULT, .data.tn##_data = __VA_ARGS__}
#define YVAR_UNDEFINED() _YVAR_INIT(YVAR_TYPE_UNDEFINED, yundefined, 0)
#define YVAR_INT8(d) _YVAR_INIT(YVAR_TYPE_INT8, yint8, (d))
#define YVAR_INT16(d) _YVAR_INIT(YVAR_TYPE_INT16, yint16, (d))
#define YVAR_CSTR(d) _YVAR_INIT(YVAR_TYPE_CSTR, ycstr, YCSTR((d)))
#define YVAR_ARRAY(d) _YVAR_INIT(YVAR_TYPE_ARRAY, yarray, {.size = sizeof((d)) / sizeof(yvar_t), .yvars = (d)})

#define YMAP_CREATE(k, v) {.keys = k, .values = v}
#define ymap_get(map, k) _ymap_get(&(map), &(k))

#define yvar_is_array(yvar) _yvar_is_array(&(yvar))
#define yvar_is_undefined(yvar) _yvar_is_undefined(&(yvar))
#define yvar_is_equal(lhs, rhs) _yvar_is_equal(&(lhs), &(rhs))
#define yvar_array_get(yvar, i) _yvar_array_get(&(yvar), (i))
#define yvar_array_size(yvar) _yvar_array_size(&(yvar))
#define yvar_has_option(yvar, opt) _yvar_has_option(&(yvar), (opt))
#define yvar_assign(lhs, rhs) _yvar_assign(&(lhs), &(rhs))

#define ymap_create(map, key, value) _ymap_create(&(map), &(key), &(value))
#define ymap_create_sorted(map, key, value) _ymap_create_sorted(&(map), &(key), &(value))

// if C99 is enabled, declare variable in for loop
#if (defined(YUKI_CONSTANTS_C99_ENABLED))

#   define YVAR_FOREACH(arr, value) \
    if (!yvar_is_array(arr)) { \
    } else \
        for (yvar_t *value = arr.data.yarray_data.yvars, \
            *end = arr.data.yarray_data.yvars + arr.data.yarray_data.size; \
            value != end; value++)

#   define YMAP_FOREACH(map, key, value) \
    ysize_t _YVAR_TEMP_VARIABLE(index##key, __LINE__) = 0; \
    if (!yvar_array_size(map.keys)) { \
    } else \
        for (yvar_t *key = map.keys.data.yarray_data.yvars, \
            *value = yvar_array_get(map.values, 0), \
            *end = map.keys.data.yarray_data.yvars + map.keys.data.yarray_data.size; \
            key != end; \
            key++, \
            value = yvar_array_get(map.values, ++_YVAR_TEMP_VARIABLE(index##key, __LINE__)))

#else // C99 is not enabled

#   define YVAR_FOREACH(arr, value) \
    yvar_t * value; \
    yvar_t * _YVAR_TEMP_VARIABLE(end##value, __LINE__); \
    if (!yvar_is_array(arr)) { \
    } else \
        for (value = arr.data.yarray_data.yvars, \
            _YVAR_TEMP_VARIABLE(end##value, __LINE__) = \
                arr.data.yarray_data.yvars + arr.data.yarray_data.size; \
            value != _YVAR_TEMP_VARIABLE(end##value, __LINE__); value++)

#   define YMAP_FOREACH(map, key, value) \
    yvar_t * key; \
    yvar_t * value; \
    yvar_t * _YVAR_TEMP_VARIABLE(end##key, __LINE__); \
    ysize_t _YVAR_TEMP_VARIABLE(index##key, __LINE__); \
    if (!yvar_array_size(map.keys)) { \
    } else \
        for (_YVAR_TEMP_VARIABLE(index##key, __LINE__) = 0, \
            key = map.keys.data.yarray_data.yvars, \
            value = yvar_array_get(map.values, 0), \
            _YVAR_TEMP_VARIABLE(end##key, __LINE__) = \
                map.keys.data.yarray_data.yvars + map.keys.data.yarray_data.size; \
            key != _YVAR_TEMP_VARIABLE(end##key, __LINE__); \
            key++, \
            value = yvar_array_get(map.values, ++_YVAR_TEMP_VARIABLE(index##key, __LINE__)))

#endif

typedef enum _YVAR_TYPE {
    YVAR_TYPE_UNDEFINED = 0,
    YVAR_TYPE_BOOL,
    YVAR_TYPE_INT8,
    YVAR_TYPE_UINT8,
    YVAR_TYPE_INT16,
    YVAR_TYPE_UINT16,
    YVAR_TYPE_INT32,
    YVAR_TYPE_UINT32,
    YVAR_TYPE_CSTR,
    YVAR_TYPE_STR,
    YVAR_TYPE_ARRAY,
} YVAR_TYPE;

typedef enum _YVAR_OPTIONS {
    YVAR_OPTION_DEFAULT = 0,
    YVAR_OPTION_HOLD_RESOURCE = 0x1, // need to free memory
    YVAR_OPTION_SORTED = 0x2, // array is sorted
} YVAR_OPTIONS;

#define ytrue 1
#define yfalse 0

typedef int8_t ybool_t;
typedef int8_t yint8_t;
typedef uint8_t yuint8_t;
typedef int16_t yint16_t;
typedef uint16_t yuint16_t;
typedef int32_t yint32_t;
typedef uint32_t yuint32_t;
typedef size_t ysize_t;

struct _yvar_t;

typedef struct _ycstr_t {
    ysize_t size;
    const char * str;
} ycstr_t;

typedef struct _yarray_t {
    ysize_t size;
    struct _yvar_t * yvars;
} yarray_t;

typedef struct _ystr_t {
    ysize_t size;
    char * str;
} ystr_t;

typedef yuint16_t yvar_option_t;

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
        ycstr_t ycstr_data;
        ystr_t ystr_data;
        yarray_t yarray_data;
    } data;
} yvar_t;

// a very simple & stupic 'map'. note: it's not based on tree. it's array.
typedef struct _ymap_t {
    yvar_t keys;
    yvar_t values;
} ymap_t;

inline ybool_t _yvar_is_array(const yvar_t * pyvar)
{
    return pyvar->type == YVAR_TYPE_ARRAY;
}

inline ybool_t _yvar_is_undefined(const yvar_t * pyvar)
{
    return pyvar->type == YVAR_TYPE_UNDEFINED;
}

inline ybool_t _yvar_is_equal(const yvar_t * plhs, const yvar_t * prhs)
{
    if (plhs == prhs) {
        return ytrue;
    }
    
    if (!plhs || !prhs || plhs->type != prhs->type) {
        return yfalse;
    }
    
    switch (plhs->type) {
        case YVAR_TYPE_BOOL:
            return plhs->data.ybool_data == prhs->data.ybool_data;
        case YVAR_TYPE_INT8:
            return plhs->data.yint8_data == prhs->data.yint8_data;
        case YVAR_TYPE_UINT8:
            return plhs->data.yuint8_data == prhs->data.yuint8_data;
        case YVAR_TYPE_INT16:
            return plhs->data.yint16_data == prhs->data.yint16_data;
        case YVAR_TYPE_UINT16:
            return plhs->data.yuint16_data == prhs->data.yuint16_data;
        case YVAR_TYPE_INT32:
            return plhs->data.yint32_data == prhs->data.yint32_data;
        case YVAR_TYPE_UINT32:
            return plhs->data.yuint32_data == prhs->data.yuint32_data;
        case YVAR_TYPE_CSTR:
            return plhs->data.ycstr_data.size == prhs->data.ycstr_data.size
                && !strcmp(plhs->data.ycstr_data.str, prhs->data.ycstr_data.str);
        case YVAR_TYPE_ARRAY:
            // TODO: implement it
            return yfalse;
        default:
            return yfalse;
    }
}

inline yvar_t * _yvar_array_get(const yvar_t * pyvar, size_t i)
{
    static yvar_t undefined = YVAR_UNDEFINED();

    if (!yvar_is_array(*pyvar)) {
        return &undefined;
    }

    const yarray_t * arr = &pyvar->data.yarray_data;
    
    if (i >= arr->size) {
        return &undefined;
    }
    
    return &arr->yvars[i];
}

inline ysize_t _yvar_array_size(const yvar_t * pyvar)
{
    if (!yvar_is_array(*pyvar)) {
        return 0;
    }
    
    return pyvar->data.yarray_data.size;
}

inline ybool_t _yvar_has_option(const yvar_t * pyvar, yvar_option_t option)
{
    return pyvar->options & option;
}

inline ybool_t _yvar_assign(yvar_t * lhs, const yvar_t * rhs)
{
    if (!lhs) {
        return yfalse;
    }
    
    if (!rhs) {
        yvar_t undefined = YVAR_UNDEFINED();
        *lhs = undefined;
    }
    
    *lhs = *rhs;
    return ytrue;
}

inline yvar_t * _ymap_get(const ymap_t * map, const yvar_t * key)
{
    static yvar_t undefined = YVAR_UNDEFINED();
    ysize_t i = 0;
    
    if (!yvar_is_array(map->values)) {
        return &undefined;
    }
    
    YVAR_FOREACH(map->keys, value) {
        if (yvar_is_equal(*value, *key)) {
            return yvar_array_get(map->values, i);
        }
        
        i++;
    }
    
    return &undefined;
}

inline ymap_t * _ymap_create(ymap_t * map, yvar_t * keys, yvar_t * values)
{
    yvar_assign(map->keys, *keys);
    yvar_assign(map->values, *values);
    return map;
}

#ifdef __cplusplus
}
#endif

#endif
