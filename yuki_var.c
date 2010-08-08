#include <string.h>
#include <stdio.h>

#include "yuki.h"

static ybool_t _ybool_to_str(ybool_t ybool, char * output, ysize_t size)
{
    if (!output || !size) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    char buf[16];
    int n = snprintf(buf, sizeof(buf), ybool? "true": "false");

    if (n < 0) {
        YUKI_LOG_FATAL("snprintf fails with err %d", n);
        return yfalse;
    }

    if (n >= size) {
        YUKI_LOG_WARNING("buffer length is too small. [required: %d] [actual: %u]", n + 1, size);
        return yfalse;
    }

    strncpy(output, buf, n + 1);
    return ytrue;
}

#define _YUKI_INT_TYPE_TO_STR_FUNCTION(t, s) \
    static ybool_t _##t##_to_str(t##_t t, char * output, ysize_t size) \
    { \
        if (!output || !size) { \
            YUKI_LOG_FATAL("invalid param"); \
            return yfalse; \
        } \
\
        char buf[32]; \
        int n = snprintf(buf, sizeof(buf), s, t); \
\
        if (n < 0) { \
            YUKI_LOG_FATAL("snprintf fails with err %d", n); \
            return yfalse; \
        } \
\
        if (n >= size) { \
            YUKI_LOG_WARNING("buffer length is too small. [required: %d] [actual: %u]", n + 1, size); \
            return yfalse; \
        } \
\
        strncpy(output, buf, n + 1); \
        return ytrue; \
    }

_YUKI_INT_TYPE_TO_STR_FUNCTION(yint8,   "%d")
_YUKI_INT_TYPE_TO_STR_FUNCTION(yuint8,  "%u")
_YUKI_INT_TYPE_TO_STR_FUNCTION(yint16,  "%d")
_YUKI_INT_TYPE_TO_STR_FUNCTION(yuint16, "%u")
_YUKI_INT_TYPE_TO_STR_FUNCTION(yint32,  "%" PRId32)
_YUKI_INT_TYPE_TO_STR_FUNCTION(yuint32, "%" PRIu32)
_YUKI_INT_TYPE_TO_STR_FUNCTION(yint64,  "%" PRId64)
_YUKI_INT_TYPE_TO_STR_FUNCTION(yuint64, "%" PRIu64)

static ybool_t _ycstr_to_str(const ycstr_t * ycstr, char * output, ysize_t size)
{
    if (!ycstr || !output || !size) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (ycstr->size > size) {
        YUKI_LOG_WARNING("buffer length is too small. [required: %u] [actual: %u]", ycstr->size, size);
        return yfalse;
    }

    strncpy(output, ycstr->str, ycstr->size);
    return ytrue;
}

ybool_t _yvar_get_bool(const yvar_t * yvar, ybool_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            *output = yfalse;
            break;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            *output = yvar->data.yint8_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_INT16:
            *output = yvar->data.yint16_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_INT32:
            *output = yvar->data.yint32_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_UINT32:
            *output = yvar->data.yuint32_data? ytrue: yfalse;
            break;
        case YVAR_TYPE_CSTR:
            *output = yvar->data.ycstr_data.size? ytrue: yfalse;
            break;
        case YVAR_TYPE_STR:
            *output = yvar->data.ystr_data.size? ytrue: yfalse;
            break;
        case YVAR_TYPE_ARRAY:
            *output = yvar->data.yarray_data.size? ytrue: yfalse;
            break;
        case YVAR_TYPE_LIST:
            *output = yvar->data.ylist_data.head? yfalse: ytrue;
            break;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_int8(const yvar_t * yvar, yint8_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to int8");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            if (yvar->data.yuint8_data > YUKI_MAX_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            if (yvar->data.yint16_data > YUKI_MAX_INT8_VALUE || yvar->data.yint16_data < YUKI_MIN_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            if (yvar->data.yuint16_data > YUKI_MAX_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data > YUKI_MAX_INT8_VALUE || yvar->data.yint32_data < YUKI_MIN_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            if (yvar->data.yuint32_data > YUKI_MAX_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_INT8_VALUE || yvar->data.yint64_data < YUKI_MIN_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_INT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint8_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to int8");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to int8");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to int8");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to int8");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_uint8(const yvar_t * yvar, yuint8_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to uint8");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = (yuint8_t)yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            if (yvar->data.yint8_data < YUKI_MIN_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            if (yvar->data.yint16_data > YUKI_MAX_UINT8_VALUE || yvar->data.yint16_data < YUKI_MIN_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            if (yvar->data.yuint16_data > YUKI_MAX_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data > YUKI_MAX_UINT8_VALUE || yvar->data.yint32_data < YUKI_MIN_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            if (yvar->data.yuint32_data > YUKI_MAX_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_UINT8_VALUE || yvar->data.yint64_data < YUKI_MIN_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_UINT8_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint8_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to uint8");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to uint8");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to uint8");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to uint8");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_int16(const yvar_t * yvar, yint16_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to int16");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = (yint16_t)yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            *output = yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            if (yvar->data.yuint16_data > YUKI_MAX_INT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint16_t)yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data > YUKI_MAX_INT16_VALUE || yvar->data.yint32_data < YUKI_MIN_INT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint16_t)yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            if (yvar->data.yuint32_data > YUKI_MAX_INT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint16_t)yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_INT16_VALUE || yvar->data.yint64_data < YUKI_MIN_INT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint16_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_INT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint16_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to int16");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to int16");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to int16");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to int16");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_uint16(const yvar_t * yvar, yuint16_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to uint16");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            if (yvar->data.yint8_data < YUKI_MIN_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            if (yvar->data.yint16_data < YUKI_MIN_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint16_t)yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data > YUKI_MAX_UINT16_VALUE || yvar->data.yint32_data < YUKI_MIN_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint16_t)yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            if (yvar->data.yuint32_data > YUKI_MAX_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint16_t)yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_UINT16_VALUE || yvar->data.yint64_data < YUKI_MIN_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint16_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_UINT16_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint16_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to uint16");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to uint16");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to uint16");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to uint16");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_int32(const yvar_t * yvar, yint32_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to int32");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            *output = yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            *output = yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            if (yvar->data.yuint32_data > YUKI_MAX_INT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint32_t)yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_INT32_VALUE || yvar->data.yint64_data < YUKI_MIN_INT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint32_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_INT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint32_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to int32");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to int32");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to int32");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to int32");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_uint32(const yvar_t * yvar, yuint32_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to uint32");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            if (yvar->data.yint8_data < YUKI_MIN_UINT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            if (yvar->data.yint16_data < YUKI_MIN_UINT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data < YUKI_MIN_UINT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint32_t)yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            *output = yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data > YUKI_MAX_UINT32_VALUE || yvar->data.yint64_data < YUKI_MIN_UINT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint32_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_UINT32_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint32_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to uint32");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to uint32");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to uint32");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to uint32");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_int64(const yvar_t * yvar, yint64_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to int64");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            *output = yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            *output = yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            *output = yvar->data.yuint32_data;
            break;
        case YVAR_TYPE_INT64:
            *output = yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            if (yvar->data.yuint64_data > YUKI_MAX_INT64_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yint64_t)yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to int64");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to int64");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to int64");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to int64");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_uint64(const yvar_t * yvar, yuint64_t * output)
{
    if (!yvar || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to uint64");
            return yfalse;
        case YVAR_TYPE_BOOL:
            *output = yvar->data.ybool_data;
            break;
        case YVAR_TYPE_INT8:
            if (yvar->data.yint8_data < YUKI_MIN_UINT64_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint8_data;
            break;
        case YVAR_TYPE_UINT8:
            *output = yvar->data.yuint8_data;
            break;
        case YVAR_TYPE_INT16:
            if (yvar->data.yint16_data < YUKI_MIN_UINT64_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint16_data;
            break;
        case YVAR_TYPE_UINT16:
            *output = yvar->data.yuint16_data;
            break;
        case YVAR_TYPE_INT32:
            if (yvar->data.yint32_data < YUKI_MIN_UINT64_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = yvar->data.yint32_data;
            break;
        case YVAR_TYPE_UINT32:
            *output = yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_INT64:
            if (yvar->data.yint64_data < YUKI_MIN_UINT64_VALUE) {
                YUKI_LOG_DEBUG("value is overflow");
                return yfalse;
            }

            *output = (yuint64_t)yvar->data.yint64_data;
            break;
        case YVAR_TYPE_UINT64:
            *output = yvar->data.yuint64_data;
            break;
        case YVAR_TYPE_CSTR:
            YUKI_LOG_DEBUG("cstr cannot be converted to uint64");
            return yfalse;
        case YVAR_TYPE_STR:
            YUKI_LOG_DEBUG("str cannot be converted to uint64");
            return yfalse;
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to uint64");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to uint64");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

ybool_t _yvar_get_str(const yvar_t * yvar, char * output, ysize_t size)
{
    return _yvar_get_cstr(yvar, output, size);
}

ybool_t _yvar_get_cstr(const yvar_t * yvar, char * output, ysize_t size)
{
    if (!yvar || !output || !size) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            YUKI_LOG_DEBUG("undefined cannot be converted to str");
            return yfalse;
        case YVAR_TYPE_BOOL:
            return _ybool_to_str(yvar->data.ybool_data, output, size);
        case YVAR_TYPE_INT8:
            return _yint8_to_str(yvar->data.yint8_data, output, size);
        case YVAR_TYPE_UINT8:
            return _yuint8_to_str(yvar->data.yuint8_data, output, size);
        case YVAR_TYPE_INT16:
            return _yint16_to_str(yvar->data.yint16_data, output, size);
        case YVAR_TYPE_UINT16:
            return _yuint16_to_str(yvar->data.yuint16_data, output, size);
        case YVAR_TYPE_INT32:
            return _yint32_to_str(yvar->data.yint32_data, output, size);
        case YVAR_TYPE_UINT32:
            return _yuint32_to_str(yvar->data.yuint32_data, output, size);
        case YVAR_TYPE_INT64:
            return _yint64_to_str(yvar->data.yint64_data, output, size);
        case YVAR_TYPE_UINT64:
            return _yuint64_to_str(yvar->data.yuint64_data, output, size);
        case YVAR_TYPE_CSTR:
        case YVAR_TYPE_STR:
            return _ycstr_to_str(&yvar->data.ycstr_data, output, size);
        case YVAR_TYPE_ARRAY:
            YUKI_LOG_DEBUG("array cannot be converted to uint64");
            return yfalse;
        case YVAR_TYPE_LIST:
            YUKI_LOG_DEBUG("list cannot be converted to uint64");
            return yfalse;
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return yfalse;
    }
    
    return ytrue;
}

/**
 * count of var. the symantic is similar to the count() in PHP.
 * only count elements in array or list.
 * one exception is undefined var. this function always returns 0 for it.
 */
ysize_t _yvar_count(const yvar_t * yvar)
{
    if (!yvar) {
        YUKI_LOG_FATAL("invalid param");
        return 0;
    }

    switch (yvar->type) {
        case YVAR_TYPE_UNDEFINED:
            return 0;
        case YVAR_TYPE_BOOL:
        case YVAR_TYPE_INT8:
        case YVAR_TYPE_UINT8:
        case YVAR_TYPE_INT16:
        case YVAR_TYPE_UINT16:
        case YVAR_TYPE_INT32:
        case YVAR_TYPE_UINT32:
        case YVAR_TYPE_CSTR:
        case YVAR_TYPE_STR:
            return 1;
        case YVAR_TYPE_ARRAY:
            return yvar->data.yarray_data.size;
        case YVAR_TYPE_LIST:
        {
            ysize_t cnt = 0;

            FOREACH_YVAR_LIST(*yvar, value) {
                cnt++;
            }

            return cnt;
        }
        default:
            YUKI_LOG_FATAL("impossible type value %d", yvar->type);
            return 0;
    }
}

ybool_t _yvar_is_equal(const yvar_t * plhs, const yvar_t * prhs)
{
    if (plhs == prhs) {
        return ytrue;
    }

    if (!plhs || !prhs) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (plhs->type != prhs->type) {
        YUKI_LOG_DEBUG("different type");
        return yfalse;
    }

    switch (plhs->type) {
        case YVAR_TYPE_UNDEFINED:
            return ytrue;
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
        case YVAR_TYPE_INT64:
            return plhs->data.yint64_data == prhs->data.yint64_data;
        case YVAR_TYPE_UINT64:
            return plhs->data.yuint64_data == prhs->data.yuint64_data;
        case YVAR_TYPE_CSTR:
        case YVAR_TYPE_STR:
            if (plhs->data.ycstr_data.size != prhs->data.ycstr_data.size) {
                return yfalse;
            }

            if (plhs->data.ycstr_data.str == prhs->data.ycstr_data.str) {
                return ytrue;
            }

            if (!plhs->data.ycstr_data.str || !prhs->data.ycstr_data.str) {
                return yfalse;
            }

            return !strcmp(plhs->data.ycstr_data.str, prhs->data.ycstr_data.str);
        case YVAR_TYPE_ARRAY:
        {
            ysize_t lhs_cnt = yvar_count(*plhs);
            ysize_t rhs_cnt = yvar_count(*prhs);

            if (lhs_cnt != rhs_cnt) {
                return yfalse;
            }

            ysize_t cnt;
            yvar_t lhs_var = YVAR_EMPTY();
            yvar_t rhs_var = YVAR_EMPTY();
            for (cnt = 0;
                cnt < lhs_cnt 
                    && yvar_array_get(*plhs, cnt, lhs_var)
                    && yvar_array_get(*prhs, cnt, rhs_var)
                    && yvar_is_equal(lhs_var, rhs_var);
                cnt++) {
                // nothing
            }

            return cnt == lhs_cnt;
        }
        case YVAR_TYPE_LIST:
        {
            ylist_node_t * lhs_head = plhs->data.ylist_data.head;
            ylist_node_t * lhs_tail = plhs->data.ylist_data.tail;
            ylist_node_t * rhs_head = prhs->data.ylist_data.head;
            ylist_node_t * rhs_tail = prhs->data.ylist_data.tail;

            while (lhs_head != lhs_tail && rhs_head != rhs_tail) {
                if (!yvar_is_equal(lhs_head->yvar, rhs_head->yvar)) {
                    return yfalse;
                }

                lhs_head = lhs_head->next;
                rhs_head = rhs_head->next;
            }

            return lhs_head == lhs_tail && rhs_head == rhs_tail;
        }
        default:
            YUKI_LOG_FATAL("impossible type value %d", plhs->type);
            return yfalse;
    }
}

ybool_t _yvar_array_get(const yvar_t * array, size_t index, yvar_t * output)
{
    if (!array || !output) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    static yvar_t undefined = YVAR_UNDEFINED();

    if (!yvar_is_array(*array)) {
        YUKI_LOG_DEBUG("var is not array");
        return yfalse;
    }

    const yarray_t * arr = &array->data.yarray_data;

    if (index >= arr->size) {
        YUKI_LOG_DEBUG("out of bound. [index: %u]", index);
        return yvar_assign(*output, undefined);
    }

    return yvar_assign(*output, arr->yvars[index]);
}

ysize_t _yvar_array_size(const yvar_t * pyvar)
{
    if (!yvar_is_array(*pyvar)) {
        return 0;
    }

    return pyvar->data.yarray_data.size;
}

ybool_t _yvar_assign(yvar_t * lhs, const yvar_t * rhs)
{
    if (!lhs || !rhs) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (yvar_has_option(*lhs, YVAR_OPTION_READONLY)) {
        YUKI_LOG_DEBUG("left hand side value is readonly");
        return yfalse;
    }

    *lhs = *rhs;
    return ytrue;
}

ybool_t _yvar_clone(yvar_t * new_var, const yvar_t * old_var)
{
    if (!new_var || !old_var) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (yvar_has_option(*new_var, YVAR_OPTION_READONLY)) {
        YUKI_LOG_FATAL("new var must be modifiable");
        return yfalse;
    }
    
    if (yvar_has_option(*old_var, YVAR_OPTION_HOLD_RESOURCE)) {
        return yvar_assign(*new_var, *old_var);
    }

    // TODO: recursively enum elements in old var and allocate new resouce to clone value
    return yfalse; // TODO: change it
}

ybool_t _ymap_get(const ymap_t * map, const yvar_t * key, yvar_t * value)
{
    if (!map || !key || !value) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    static yvar_t undefined = YVAR_UNDEFINED();

    if (!yvar_is_array(map->values) || !yvar_is_array(map->values)) {
        YUKI_LOG_FATAL("map keys or values is not an array. why?");
        return yfalse;
    }

    // TODO: for sorted map, use binary search
    ysize_t i = 0;
    FOREACH_YVAR_ARRAY(map->keys, v) {
        if (yvar_is_equal(*v, *key)) {
            yvar_t array_var = YVAR_EMPTY();
            yvar_array_get(map->values, i, array_var);
            return yvar_assign(*value, array_var);
        }

        i++;
    }

    YUKI_LOG_DEBUG("key is not found");
    yvar_assign(*value, undefined);
    return yfalse;
}

ybool_t _ymap_create(ymap_t * map, yvar_t * keys, yvar_t * values)
{
    if (!yvar_assign(map->keys, *keys)) {
        YUKI_LOG_DEBUG("fail to assign keys");
        return yfalse;
    }

    return yvar_assign(map->values, *values);
}
