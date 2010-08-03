#ifndef _YUKI_TYPE_H_
#define _YUKI_TYPE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ytrue 1
#define yfalse 0

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
    YVAR_OPTION_READONLY = 0x1,
    YVAR_OPTION_HOLD_RESOURCE = 0x2, // need to free memory
    YVAR_OPTION_SORTED = 0x4, // array is sorted
} YVAR_OPTIONS;

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

typedef struct _ytable_t {
    yvar_t table_name;
    yvar_t condition;
    yvar_t params;
    yvar_t hash_key;
    yvar_t sql;
} ytable_t;

#ifdef __cplusplus
}
#endif

#endif
