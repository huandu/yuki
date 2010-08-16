#ifndef _YUKI_VAR_H_
#define _YUKI_VAR_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YUKI_VAR_VERSION 0x1

#define _YVAR_TEMP_VARIABLE_REAL(p, s) __yvar_temp##p##s
#define _YVAR_TEMP_VARIABLE(p, s) _YVAR_TEMP_VARIABLE_REAL(p, s)
#define _YVAR_INIT(t, tn, ...) {.type = (t), .version = YUKI_VAR_VERSION, .options = YVAR_OPTION_DEFAULT, .data.tn##_data = __VA_ARGS__}
#define _YVAR_INIT_WITH_OPTION(t, option, tn, ...) {.type = (t), .version = YUKI_VAR_VERSION, .options = (option), .data.tn##_data = __VA_ARGS__}

#define YVAR_UNDEFINED() _YVAR_INIT_WITH_OPTION(YVAR_TYPE_UNDEFINED, YVAR_OPTION_READONLY, yundefined, 0)
#define YVAR_EMPTY() {0}
#define YVAR_BOOL(d) _YVAR_INIT(YVAR_TYPE_BOOL, ybool, (d))
#define YVAR_INT8(d) _YVAR_INIT(YVAR_TYPE_INT8, yint8, (d))
#define YVAR_UINT8(d) _YVAR_INIT(YVAR_TYPE_UINT8, yuint8, (d))
#define YVAR_INT16(d) _YVAR_INIT(YVAR_TYPE_INT16, yint16, (d))
#define YVAR_UINT16(d) _YVAR_INIT(YVAR_TYPE_UINT16, yuint16, (d))
#define YVAR_INT32(d) _YVAR_INIT(YVAR_TYPE_INT32, yint32, (d))
#define YVAR_UINT32(d) _YVAR_INIT(YVAR_TYPE_UINT32, yuint32, (d))
#define YVAR_INT64(d) _YVAR_INIT(YVAR_TYPE_INT64, yint64, (d))
#define YVAR_UINT64(d) _YVAR_INIT(YVAR_TYPE_UINT64, yuint64, (d))
#define YVAR_CSTR(d) _YVAR_INIT(YVAR_TYPE_CSTR, ycstr, YCSTR((d)))
#define YVAR_STR() _YVAR_INIT(YVAR_TYPE_STR, ystr, {0})
#define YVAR_ARRAY(d) _YVAR_INIT(YVAR_TYPE_ARRAY, yarray, {.size = sizeof((d)) / sizeof(yvar_t), .yvars = (d)})
#define YVAR_LIST() _YVAR_INIT(YVAR_TYPE_LIST, ylist, {0})
#define YVAR_MAP(k, v) _YVAR_INIT(YVAR_TYPE_MAP, ymap, {&(k), &(v)})

// following macros is for C++ compatible
// NOTE: don't use them in pure C project. use upper case macro instead.
// {{{
#define _YVAR_INIT_FOR_CPP(yvar, t, tn, d) do { \
        yvar_t * pointer = &(yvar); \
        pointer->type = (t); \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_DEFAULT; \
        pointer->data.tn##_data = (d); \
    } while (0)
#define _YVAR_INIT_WITH_OPTION_FOR_CPP(yvar, t, option, tn, d) do { \
        yvar_t * pointer = &(yvar); \
        pointer->type = (t); \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = (option); \
        pointer->data.tn##_data = (d); \
    } while (0)

#define yvar_undefined(yvar) _YVAR_INIT_WITH_OPTION_FOR_CPP(yvar, YVAR_TYPE_UNDEFINED, YVAR_OPTION_DEFAULT, yundefined, 0)
#define yvar_bool(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_BOOL, ybool, (d))
#define yvar_int8(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_INT8, yint8, (d))
#define yvar_uint8(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_UINT8, yuint8, (d))
#define yvar_int16(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_INT16, yint16, (d))
#define yvar_uint16(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_UINT16, yuint16, (d))
#define yvar_int32(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_INT32, yint32, (d))
#define yvar_uint32(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_UINT32, yuint32, (d))
#define yvar_int64(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_INT64, yint64, (d))
#define yvar_uint64(yvar, d) _YVAR_INIT_FOR_CPP(yvar, YVAR_TYPE_UINT64, yuint64, (d))
#define yvar_cstr(yvar, d) do { \
        yvar_t * pointer = &(yvar); \
        ycstr_t str = {sizeof((d)) - 1, (d)}; \
        pointer->type = YVAR_TYPE_CSTR; \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_DEFAULT; \
        pointer->data.ycstr_data = str; \
    } while (0)
#define yvar_str(yvar) do { \
        yvar_t * pointer = &(yvar); \
        ystr_t str = {0}; \
        pointer->type = YVAR_TYPE_STR; \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_READONLY; \
        pointer->data.ystr_data = str; \
    } while (0)
#define yvar_array(yvar, d) do { \
        yvar_t * pointer = &(yvar); \
        yarray_t array = {sizeof((d)) / sizeof(yvar_t), (d)}; \
        pointer->type = YVAR_TYPE_ARRAY; \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_DEFAULT; \
        pointer->data.yarray_data = array; \
    } while (0)
#define yvar_list(yvar) do { \
        yvar_t * pointer = &(yvar); \
        ylist_t list = {0}; \
        pointer->type = YVAR_TYPE_LIST; \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_DEFAULT; \
        pointer->data.ylist_data = list; \
    } while (0)
#define yvar_map(yvar, k, v) do { \
        yvar_t * pointer = &(yvar); \
        ymap_t map = {&(k), &(v)}; \
        pointer->type = YVAR_TYPE_MAP; \
        pointer->version = YUKI_VAR_VERSION; \
        pointer->options = YVAR_OPTION_DEFAULT; \
        pointer->data.ymap_data = map; \
    } while (0)
// }}}

#define _YVAR_IS_TYPE(yvar, t) ((yvar).type == (t))
#define yvar_is_undefined(yvar) _YVAR_IS_TYPE((yvar), YVAR_TYPE_UNDEFINED)
#define yvar_is_bool(yvar)      _YVAR_IS_TYPE((yvar), YVAR_TYPE_BOOL)
#define yvar_is_int8(yvar)      _YVAR_IS_TYPE((yvar), YVAR_TYPE_INT8)
#define yvar_is_uint8(yvar)     _YVAR_IS_TYPE((yvar), YVAR_TYPE_UINT8)
#define yvar_is_int16(yvar)     _YVAR_IS_TYPE((yvar), YVAR_TYPE_INT16)
#define yvar_is_uint16(yvar)    _YVAR_IS_TYPE((yvar), YVAR_TYPE_UINT16)
#define yvar_is_int32(yvar)     _YVAR_IS_TYPE((yvar), YVAR_TYPE_INT32)
#define yvar_is_uint32(yvar)    _YVAR_IS_TYPE((yvar), YVAR_TYPE_UINT32)
#define yvar_is_int64(yvar)     _YVAR_IS_TYPE((yvar), YVAR_TYPE_INT64)
#define yvar_is_uint64(yvar)    _YVAR_IS_TYPE((yvar), YVAR_TYPE_UINT64)
#define yvar_is_cstr(yvar)      _YVAR_IS_TYPE((yvar), YVAR_TYPE_CSTR)
#define yvar_is_str(yvar)       _YVAR_IS_TYPE((yvar), YVAR_TYPE_STR)
#define yvar_is_array(yvar)     _YVAR_IS_TYPE((yvar), YVAR_TYPE_ARRAY)
#define yvar_is_list(yvar)      _YVAR_IS_TYPE((yvar), YVAR_TYPE_LIST)
#define yvar_is_map(yvar)       _YVAR_IS_TYPE((yvar), YVAR_TYPE_MAP)

#define yvar_get_bool(yvar, output) _yvar_get_bool(&(yvar), &(output))
#define yvar_get_int8(yvar, output) _yvar_get_int8(&(yvar), &(output))
#define yvar_get_uint8(yvar, output) _yvar_get_uint8(&(yvar), &(output))
#define yvar_get_int16(yvar, output) _yvar_get_int16(&(yvar), &(output))
#define yvar_get_uint16(yvar, output) _yvar_get_uint16(&(yvar), &(output))
#define yvar_get_int32(yvar, output) _yvar_get_int32(&(yvar), &(output))
#define yvar_get_uint32(yvar, output) _yvar_get_uint32(&(yvar), &(output))
#define yvar_get_int64(yvar, output) _yvar_get_int64(&(yvar), &(output))
#define yvar_get_uint64(yvar, output) _yvar_get_uint64(&(yvar), &(output))
#define yvar_get_cstr(yvar, output, size) _yvar_get_cstr(&(yvar), (output), (size))
#define yvar_get_str(yvar, output, size) _yvar_get_str(&(yvar), (output), (size))

#define yvar_count(yvar) _yvar_count(&(yvar))
#define yvar_is_equal(lhs, rhs) _yvar_is_equal(&(lhs), &(rhs))
#define yvar_compare(lhs, rhs) _yvar_compare(&(lhs), &(rhs))

#define yvar_str_strlen(yvar) _yvar_cstr_strlen(&(yvar))
#define yvar_cstr_strlen(yvar) _yvar_cstr_strlen(&(yvar))

#define yvar_array_get(yvar, index, output) _yvar_array_get(&(yvar), (index), &(output))
#define yvar_array_size(yvar) _yvar_array_size(&(yvar))

#define yvar_list_push_back(yvar, node) _yvar_list_push_back(&(yvar), &(node))

#define yvar_map_get(map, k, v) _yvar_map_get(&(map), &(k), &(v))

#define yvar_assign(lhs, rhs) _yvar_assign(&(lhs), &(rhs))
#define yvar_clone(new_var, old_var) _yvar_clone(&(new_var), &(old_var))
#define yvar_pin(new_var, old_var) _yvar_pin(&(new_var), &(old_var))
#define yvar_unpin(yvar) _yvar_unpin((yvar))
#define yvar_memzero(yvar) _yvar_memzero(&(yvar))
#define yvar_unset(yvar) yvar_memzero(yvar)

#define yvar_has_option(yvar, opt) ((yvar).options & (opt))
#define yvar_set_option(yvar, opt) ((yvar).options |= (opt))
#define yvar_reset_option(yvar, opt) ((yvar).options &= ~(opt))

/** get read/write reference of internal string buffer of cstr var. */
#define yvar_cstr_buffer(yvar) ((yvar).data.ycstr_data.str)
/** get read/write reference of internal string buffer of cstr var. */
#define yvar_str_buffer(yvar) ((yvar).data.ystr_data.str)

// hey friend, i don't intend to use following code to frighten you.
// but it's really too complex to implement a 'foreach' loop in C.
// if you find any issue when using these 'foreach's, please keep calm and contact me.

// if C99 is enabled, declare variable in for loop
#if (defined(YUKI_CONFIG_C99_ENABLED))
/**
 * iterate array elements in a var.
 * if var is not an array, do nothing.
 * 
 * sample code.
 * @code
 * yvar_t raw_arr = {YVAR_INT8(23), YVAR_CSTR("Hello"), YVAR_UINT32(222)};
 * yvar_t arr = YVAR_ARR(raw_arr);
 * 
 * // note: don't declare 'value' yourself. i will do this for you.
 * FOREACH_YVAR_ARRAY(arr, value) {
 *     // type of 'value' is yvar_t*. you can use it freely.
 * }
 * @endcode
 */
# define FOREACH_YVAR_ARRAY(arr, value) \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(arr); \
    if (!yvar_is_array(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach array on a non array var"); \
    } else \
        for (yvar_t *value = _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.yvars, \
            *_YVAR_TEMP_VARIABLE(end##key, __LINE__) = \
                _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.yvars + \
                _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.size; \
            value != _YVAR_TEMP_VARIABLE(end##key, __LINE__); value++)

/**
 * iterate list elements in a var.
 * if var is not an list, do nothing.
 * 
 * sample code.
 * @code
 * yvar_t list = YVAR_LIST();
 * yvar_t value1 = YVAR_INT8(23);
 * yvar_t value2 = YVAR_CSTR("Hey");
 * yvar_t value3 = YVAR_INT32(2222);
 *
 * yvar_list_push_back(value1);
 * yvar_list_push_back(value2);
 * yvar_list_push_back(value3);
 * 
 * // note: don't declare 'value' yourself. i will do this for you.
 * FOREACH_YVAR_LIST(list, value) {
 *     // type of 'value' is yvar_t*. you can use it freely.
 * }
 * @endcode
 */
# define FOREACH_YVAR_LIST(list, value) \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(list); \
    ylist_node_t * _YVAR_TEMP_VARIABLE(head##key, __LINE__) = _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ylist_data.head; \
    if (!yvar_is_list(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach list on a non list var"); \
    } else \
        for (yvar_t *value = &_YVAR_TEMP_VARIABLE(head##key, __LINE__)->yvar; \
            _YVAR_TEMP_VARIABLE(head##key, __LINE__); \
            value = &(_YVAR_TEMP_VARIABLE(head##key, __LINE__) = _YVAR_TEMP_VARIABLE(head##key, __LINE__)->next)->yvar)

/**
 * iterate map elements.
 * 
 * sample code.
 * @code
 * yvar_t raw_keys = {YVAR_INT8(23), YVAR_CSTR("Hello"), YVAR_UINT32(222)};
 * yvar_t keys = YVAR_ARR(raw_keys);
 * yvar_t raw_values = {YVAR_INT8(54), YVAR_CSTR("World"), YVAR_UINT32(438)};
 * yvar_t values = YVAR_ARR(raw_values);
 * yvar_t map = YVAR_MAP(keys, values);
 * 
 * // note: don't declare 'key' and 'value' yourself. i will do this for you.
 * FOREACH_YMAP(map, key, value) {
 *     // both type of 'key' and 'value' are yvar_t*. you can use it freely.
 * }
 * @endcode
 */
# define FOREACH_YVAR_MAP(map, key, value) \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(map); \
    yvar_t * _YVAR_TEMP_VARIABLE(keys##key, __LINE__) = \
        _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ymap_data.keys; \
    yvar_t * _YVAR_TEMP_VARIABLE(values##key, __LINE__) = \
        _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ymap_data.values; \
    if (!yvar_is_map(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__)) \
        || !yvar_is_array(*_YVAR_TEMP_VARIABLE(keys##key, __LINE__)) \
        || !yvar_is_array(*_YVAR_TEMP_VARIABLE(values##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach map on a var which is not a map var or an invalid map"); \
    } else \
        for (yvar_t *key = _YVAR_TEMP_VARIABLE(keys##key, __LINE__)->data.yarray_data.yvars, \
            *value = _YVAR_TEMP_VARIABLE(values##key, __LINE__)->data.yarray_data.yvars, \
            *_YVAR_TEMP_VARIABLE(end##key, __LINE__) = \
                key + _YVAR_TEMP_VARIABLE(keys##key, __LINE__)->data.yarray_data.size; \
            key != _YVAR_TEMP_VARIABLE(end##key, __LINE__); key++, value++)
#else // C99 is not enabled
# define FOREACH_YVAR_ARRAY(arr, value) \
    yvar_t * value; \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(arr); \
    yvar_t * _YVAR_TEMP_VARIABLE(end##key, __LINE__); \
    if (!yvar_is_array(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach array on a non array var"); \
    } else \
        for (value = _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.yvars, \
            _YVAR_TEMP_VARIABLE(end##key, __LINE__) = \
                _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.yvars + \
                _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.yarray_data.size; \
            value != _YVAR_TEMP_VARIABLE(end##key, __LINE__); value++)

# define FOREACH_YVAR_LIST(list, value) \
    yvar_t * value; \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(list); \
    ylist_node_t * _YVAR_TEMP_VARIABLE(head##key, __LINE__) = _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ylist_data.head; \
    if (!yvar_is_list(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach list on a non list var"); \
    } else \
        for (value = &_YVAR_TEMP_VARIABLE(head##key, __LINE__)->yvar; \
            _YVAR_TEMP_VARIABLE(head##key, __LINE__); \
            value = &(_YVAR_TEMP_VARIABLE(head##key, __LINE__) = _YVAR_TEMP_VARIABLE(head##key, __LINE__)->next)->yvar)

# define FOREACH_YVAR_MAP(map, key, value) \
    yvar_t *key, *value; \
    const yvar_t * _YVAR_TEMP_VARIABLE(yvar##key, __LINE__) = &(map); \
    yvar_t * _YVAR_TEMP_VARIABLE(keys##key, __LINE__) = \
        _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ymap_data.keys; \
    yvar_t * _YVAR_TEMP_VARIABLE(values##key, __LINE__) = \
        _YVAR_TEMP_VARIABLE(yvar##key, __LINE__)->data.ymap_data.values; \
    yvar_t * _YVAR_TEMP_VARIABLE(end##key, __LINE__); \
    if (!yvar_is_map(*_YVAR_TEMP_VARIABLE(yvar##key, __LINE__)) \
        || !yvar_is_array(*_YVAR_TEMP_VARIABLE(keys##key, __LINE__)) \
        || !yvar_is_array(*_YVAR_TEMP_VARIABLE(values##key, __LINE__))) { \
        YUKI_LOG_DEBUG("cannot do foreach map on a var which is not a map var or an invalid map"); \
    } else \
        for (key = _YVAR_TEMP_VARIABLE(keys##key, __LINE__)->data.yarray_data.yvars, \
            value = _YVAR_TEMP_VARIABLE(values##key, __LINE__)->data.yarray_data.yvars, \
            _YVAR_TEMP_VARIABLE(end##key, __LINE__) = \
                key + _YVAR_TEMP_VARIABLE(keys##key, __LINE__)->data.yarray_data.size; \
            key != _YVAR_TEMP_VARIABLE(end##key, __LINE__); key++, value++)
#endif

#define _YVAR_GET_FUNCTION_DECLARE(t) ybool_t _yvar_get_##t(const yvar_t * yvar, y##t##_t * output)
#define _YVAR_GET_FUNCTION_DECLARE_WITH_SIZE(t) ybool_t _yvar_get_##t(const yvar_t * yvar, char * output, ysize_t size)

_YVAR_GET_FUNCTION_DECLARE(bool);
_YVAR_GET_FUNCTION_DECLARE(int8);
_YVAR_GET_FUNCTION_DECLARE(uint8);
_YVAR_GET_FUNCTION_DECLARE(int16);
_YVAR_GET_FUNCTION_DECLARE(uint16);
_YVAR_GET_FUNCTION_DECLARE(int32);
_YVAR_GET_FUNCTION_DECLARE(uint32);
_YVAR_GET_FUNCTION_DECLARE(int64);
_YVAR_GET_FUNCTION_DECLARE(uint64);
_YVAR_GET_FUNCTION_DECLARE_WITH_SIZE(cstr);
_YVAR_GET_FUNCTION_DECLARE_WITH_SIZE(str);

ysize_t _yvar_count(const yvar_t * yvar);
ybool_t _yvar_is_equal(const yvar_t * plhs, const yvar_t * prhs);
yint8_t _yvar_compare(const yvar_t * plhs, const yvar_t * prhs);

ysize_t _yvar_cstr_strlen(const yvar_t * yvar);

ybool_t _yvar_array_get(const yvar_t * pyvar, size_t index, yvar_t * output);
ysize_t _yvar_array_size(const yvar_t * pyvar);

ybool_t _yvar_list_push_back(yvar_t * yvar, yvar_t * node);

ybool_t _yvar_map_get(const yvar_t * map, const yvar_t * key, yvar_t * value);

ybool_t _yvar_assign(yvar_t * lhs, const yvar_t * rhs);
ybool_t _yvar_clone(yvar_t ** new_var, const yvar_t * old_var);
ybool_t _yvar_pin(yvar_t ** new_var, const yvar_t * old_var);
ybool_t _yvar_unpin(yvar_t * yvar);
ybool_t _yvar_memzero(yvar_t * new_var);

#ifdef __cplusplus
}
#endif

#endif
