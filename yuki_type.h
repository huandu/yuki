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

// NOTE: keep int-like type continuous so that yvar_like_int() can be easier
#define YVAR_TYPE_INT_MIN YVAR_TYPE_BOOL
#define YVAR_TYPE_INT_MAX YVAR_TYPE_UINT64

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
    YVAR_TYPE_MAX, // max
} YVAR_TYPE;

/**
 * yuki var options. options can be combined by '|'.
 */
typedef enum _YVAR_OPTIONS {
    YVAR_OPTION_DEFAULT = 0,
    YVAR_OPTION_READONLY = 0x1, /**< yvar_assign() cannot modify readonly var */
    YVAR_OPTION_HOLD_RESOURCE = 0x2, /**< need to free memory */
    YVAR_OPTION_SORTED = 0x4, /**< array is sorted */
    YVAR_OPTION_PINNED = 0x8, /**< var is pinned. pinned var cannot be modified until upinned. */
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

typedef yvar_t yvar_map_kv_t[][2];
typedef yvar_t yvar_triple_array_t[][3];

typedef struct _ylist_node_t {
    struct _ylist_node_t * prev;
    struct _ylist_node_t * next;
    yvar_t yvar;
} ylist_node_t;

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

typedef enum _ytable_hash_method_t {
    YTABLE_HASH_METHOD_INVALID,
    YTABLE_HASH_METHOD_DEFAULT,
    YTABLE_HASH_METHOD_KEY_HASH,
    YTABLE_HASH_METHOD_MAX,
} ytable_hash_method_t;

typedef enum _ytable_verb_t {
    YTABLE_VERB_NULL = 0,
    YTABLE_VERB_SELECT,
    YTABLE_VERB_UPDATE,
    YTABLE_VERB_INSERT,
    YTABLE_VERB_DELETE,
    YTABLE_VERB_MAX,
} ytable_verb_t;

typedef enum _ytable_error_t {
    YTABLE_ERROR_SUCCESS,
    YTABLE_ERROR_INVALID_PARAM,
    YTABLE_ERROR_INVALID_VERB,
    YTABLE_ERROR_INVALID_FIELD,
    YTABLE_ERROR_INVALID_CONDITION,
    YTABLE_ERROR_CANNOT_CLONE_VAR,
    YTABLE_ERROR_CANNOT_BUILD_SQL,
    YTABLE_ERROR_CANNOT_PARSE_RESULT,
    YTABLE_ERROR_CONFLICTED_VERB,
    YTABLE_ERROR_CONNECTION,
    YTABLE_ERROR_NOT_EXPECTED_RESULT,
    YTABLE_ERROR_CANNOT_FETCH_INSERT_ID,
    YTABLE_ERROR_NOT_IMPLEMENTED,
    YTABLE_ERROR_UNKNOWN,
} ytable_error_t;

// declared in yuki_table.c to avoid dependence on <mysql.h>
struct _ytable_connection_t;

typedef struct _ytable_t {
    yvar_t * fields;
    yvar_t * conditions;
    yvar_t * hash_value;
    yvar_t * order_by;
    struct _ytable_connection_t * active_connection;
    yint32_t limit;
    yint32_t offset;
    ysize_t affected_rows;
    yvar_t sql;
    ytable_verb_t verb;
    ytable_error_t last_error;
    ysize_t ytable_index; /**< index in ytable conf. */
} ytable_t;

typedef struct _ytable_connection_thread_data_t {
    struct _ytable_connection_t * connections;
    ysize_t size;
} ytable_connection_thread_data_t;

typedef struct _ytable_table_config_t {
    const char * name;
    ysize_t name_len;
    const char * hash_key;
    yvar_t * params;
    yvar_t * connection_index;
    ytable_hash_method_t hash_method;
} ytable_table_config_t;

typedef struct _ytable_connection_config_t {
    const char * name;
    const char * host;
    const char * user;
    const char * password;
    const char * database;
    const char * character_set;
    yint32_t port;
} ytable_connection_config_t;

// cheat compiler
struct _ytable_mysql_res_t;

typedef ybool_t (*ytable_sql_validator_func)(const ytable_t * ytable);
typedef ybool_t (*ytable_sql_builder_func)(ytable_t * ytable);
typedef ybool_t (*ytable_sql_result_parser_func)(const ytable_t * ytable, struct _ytable_mysql_res_t * mysql_res, yvar_t ** result);

typedef struct _ytable_sql_buider_t {
    const char * verb;
    ytable_sql_validator_func validator;
    ytable_sql_builder_func builder;
    ytable_sql_result_parser_func parser;
} ytable_sql_builder_t;

#ifdef __cplusplus
}
#endif

#endif
