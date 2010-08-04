#ifndef _YUKI_VAR_H_
#define _YUKI_VAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define _YVAR_TEMP_VARIABLE(p, s) __yvar_temp##p##s
#define _YVAR_INIT(t, tn, ...) {.type = (t), .version = YUKI_VAR_VERSION, .options = YVAR_OPTION_DEFAULT, .data.tn##_data = __VA_ARGS__}
#define _YVAR_INIT_WITH_OPTION(t, option, tn, ...) {.type = (t), .version = YUKI_VAR_VERSION, .options = (option), .data.tn##_data = __VA_ARGS__}

#define YUKI_VAR_VERSION 1

#define YVAR_UNDEFINED() _YVAR_INIT_WITH_OPTION(YVAR_TYPE_UNDEFINED, YVAR_OPTION_READONLY, yundefined, 0)
#define YVAR_INT8(d) _YVAR_INIT(YVAR_TYPE_INT8, yint8, (d))
#define YVAR_UINT8(d) _YVAR_INIT(YVAR_TYPE_UINT8, yuint8, (d))
#define YVAR_INT16(d) _YVAR_INIT(YVAR_TYPE_INT16, yint16, (d))
#define YVAR_UINT16(d) _YVAR_INIT(YVAR_TYPE_UINT16, yuint16, (d))
#define YVAR_INT32(d) _YVAR_INIT(YVAR_TYPE_INT32, yint32, (d))
#define YVAR_UINT32(d) _YVAR_INIT(YVAR_TYPE_UINT32, yuint32, (d))
#define YVAR_CSTR(d) _YVAR_INIT(YVAR_TYPE_CSTR, ycstr, YCSTR((d)))
#define YVAR_ARRAY(d) _YVAR_INIT(YVAR_TYPE_ARRAY, yarray, {.size = sizeof((d)) / sizeof(yvar_t), .yvars = (d)})
#define YMAP_CREATE(k, v) {.keys = k, .values = v}

#define yvar_is_array(yvar) _yvar_is_array(&(yvar))
#define yvar_is_undefined(yvar) _yvar_is_undefined(&(yvar))
#define yvar_is_equal(lhs, rhs) _yvar_is_equal(&(lhs), &(rhs))
#define yvar_array_get(yvar, i) _yvar_array_get(&(yvar), (i))
#define yvar_array_size(yvar) _yvar_array_size(&(yvar))
#define yvar_has_option(yvar, opt) _yvar_has_option(&(yvar), (opt))
#define yvar_assign(lhs, rhs) _yvar_assign(&(lhs), &(rhs))
#define yvar_clone(new_var, old_var) _yvar_clone(&(new_var), &(old_var))

#define ymap_get(map, k) _ymap_get(&(map), &(k))
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

inline ybool_t _yvar_is_array(const yvar_t * pyvar);
inline ybool_t _yvar_is_undefined(const yvar_t * pyvar);
inline ybool_t _yvar_is_equal(const yvar_t * plhs, const yvar_t * prhs);

inline yvar_t * _yvar_array_get(const yvar_t * pyvar, size_t i);
inline ysize_t _yvar_array_size(const yvar_t * pyvar);

inline ybool_t _yvar_has_option(const yvar_t * pyvar, yvar_option_t option);
inline ybool_t _yvar_assign(yvar_t * lhs, const yvar_t * rhs);
inline ybool_t _yvar_clone(yvar_t * new_var, const yvar_t * old_var);

inline yvar_t * _ymap_get(const ymap_t * map, const yvar_t * key);
inline ymap_t * _ymap_create(ymap_t * map, yvar_t * keys, yvar_t * values);

#ifdef __cplusplus
}
#endif

#endif
