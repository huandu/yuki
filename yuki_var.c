#include <string.h>

#include "yuki.h"

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
    if (!lhs || yvar_has_option(*lhs, YVAR_OPTION_READONLY)) {
        return yfalse;
    }

    if (!rhs) {
        yvar_t undefined = YVAR_UNDEFINED();
        *lhs = undefined;
    }

    *lhs = *rhs;
    return ytrue;
}

inline ybool_t _yvar_clone(yvar_t * new_var, const yvar_t * old_var)
{
    if (!new_var || yvar_has_option(*new_var, YVAR_OPTION_READONLY)) {
        return yfalse;
    }

    if (!old_var) {
        yvar_t undefined = YVAR_UNDEFINED();
        return yvar_assign(*new_var, undefined);
    }
    
    if (yvar_has_option(*old_var, YVAR_OPTION_HOLD_RESOURCE)) {
        return yvar_assign(*new_var, *old_var);
    }

    // TODO: recursively enum elements in old var and allocate new resouce to clone value
    return yfalse; // TODO: change it
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
