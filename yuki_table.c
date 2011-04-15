#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include <pthread.h>
#include <mysql.h>
#include <assert.h>

#include "libconfig.h"
#include "yuki.h"

#define YTABLE_CONFIG_PATH_CONNECTIONS YUKI_CONFIG_SECTION_YTABLE "/connections"
#define YTABLE_CONFIG_PATH_TABLES      YUKI_CONFIG_SECTION_YTABLE "/tables"

#define YTABLE_CONFIG_MEMBER_NAME "name"
#define YTABLE_CONFIG_MEMBER_HOST "host"
#define YTABLE_CONFIG_MEMBER_USER "user"
#define YTABLE_CONFIG_MEMBER_PASSWORD "password"
#define YTABLE_CONFIG_MEMBER_DATABASE "database"
#define YTABLE_CONFIG_MEMBER_CHARACTER_SET "character_set"
#define YTABLE_CONFIG_MEMBER_PORT "port"
#define YTABLE_CONFIG_MEMBER_CONNECTION "connection"
#define YTABLE_CONFIG_MEMBER_HASH_KEY "hash_key"
#define YTABLE_CONFIG_MEMBER_HASH_METHOD "hash_method"
#define YTABLE_CONFIG_MEMBER_PARAMS "params"
#define YTABLE_CONFIG_MEMBER_TABLE_LENGTH "table_length"
#define YTABLE_CONFIG_MEMBER_DB_LENGTH "db_length"
#define YTABLE_CONFIG_MEMBER_DB_PREFIX "db_prefix"

#define YTABLE_CONFIG_DEFAULT_PORT 3306

#define _YTABLE_CONFIG_SETTING_STRING(config, path, var) do { \
        if (CONFIG_TRUE != config_setting_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_FATAL("'%s' element must have member '%s'", \
                YTABLE_CONFIG_PATH_CONNECTIONS, (path)); \
            return yfalse; \
        } \
        YUKI_LOG_DEBUG("'%s' is set to '%s'", (path), (var)); \
    } while (0)

#define _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_setting_lookup_string((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%s'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%s'", \
                (path), (def)); \
        } \
    } while (0)

#define _YTABLE_CONFIG_SETTING_INT_OPTIONAL(config, path, var, def) do { \
        if (CONFIG_TRUE == config_setting_lookup_int((config), (path), &(var))) { \
            YUKI_LOG_DEBUG("'%s' is set to '%d'", \
                (path), (var)); \
        } else { \
            (var) = (def); \
            YUKI_LOG_DEBUG("'%s' is set to default value '%d'", \
                (path), (def)); \
        } \
    } while (0)

#define _YTABLE_CONFIG_ESTIMATE_STRING(size, str) do { \
        (size) += ((str)? ybuffer_round_up(strlen((str)) + 1): 0); \
    } while (0)

#define _YTABLE_CONFIG_COPY_STRING(buffer, str) do { \
        if ((str)) { \
            ysize_t str_len = strlen((str)) + 1; \
            char * new_str = (char*)ybuffer_alloc((buffer), str_len); \
            YUKI_ASSERT(new_str); \
            snprintf(new_str, str_len, "%s", (str)); \
            (str) = new_str; \
        } \
    } while (0)

typedef struct _ytable_connection_t {
    MYSQL mysql;
    ybool_t connected;
} ytable_connection_t;

typedef struct _ytable_mysql_res_t {
    MYSQL_RES res;
} ytable_mysql_res_t;

static ytable_table_config_t *g_ytable_table_configs = NULL;
static ysize_t g_ytable_table_configs_count = 0;

static ytable_connection_config_t *g_ytable_connection_configs = NULL;
static ysize_t g_ytable_connection_configs_count = 0;

static ybool_t g_ytable_inited = yfalse;
static pthread_key_t g_ytable_connection_thread_key;

static yvar_t g_ytable_result_true = YVAR_BOOL(ytrue);
static yvar_t g_ytable_result_false = YVAR_BOOL(yfalse);

static ybool_t _ytable_sql_select_validator(const ytable_t * ytable);
static ybool_t _ytable_sql_update_validator(const ytable_t * ytable);
static ybool_t _ytable_sql_insert_validator(const ytable_t * ytable);
static ybool_t _ytable_sql_delete_validator(const ytable_t * ytable);

static ybool_t _ytable_sql_select_builder(ytable_t * ytable);
static ybool_t _ytable_sql_update_builder(ytable_t * ytable);
static ybool_t _ytable_sql_insert_builder(ytable_t * ytable);
static ybool_t _ytable_sql_delete_builder(ytable_t * ytable);

static ybool_t _ytable_sql_select_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result);
static ybool_t _ytable_sql_update_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result);
static ybool_t _ytable_sql_insert_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result);
static ybool_t _ytable_sql_delete_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result);

static const ytable_sql_builder_t g_ytable_sql_builder[] = {
    {"<NULL>", NULL, NULL}, // YTABLE_VERB_NULL
    {"SELECT", // YTABLE_VERB_SELECT
     &_ytable_sql_select_validator, &_ytable_sql_select_builder, &_ytable_sql_select_result_parser},
    {"UPDATE", // YTABLE_VERB_UPDATE
     &_ytable_sql_update_validator, &_ytable_sql_update_builder, &_ytable_sql_update_result_parser},
    {"INSERT", // YTABLE_VERB_INSERT
     &_ytable_sql_insert_validator, &_ytable_sql_insert_builder, &_ytable_sql_insert_result_parser},
    {"DELETE", // YTABLE_VERB_DELETE
     &_ytable_sql_delete_validator, &_ytable_sql_delete_builder, &_ytable_sql_delete_result_parser},
};

static ybool_t _ytable_config_set_hash_method(ytable_table_config_t * config, const char * hash_method, config_setting_t * setting)
{
    YUKI_ASSERT(setting);

    if (!hash_method) {
        config->hash_method = YTABLE_HASH_METHOD_DEFAULT;
        return ytrue;
    }
    if (!strcmp(hash_method, "key_hash")) {
        //all of options is optional
        yint32_t table_length = 0;
        _YTABLE_CONFIG_SETTING_INT_OPTIONAL(setting, YTABLE_CONFIG_MEMBER_TABLE_LENGTH, table_length, 0);
        yint32_t db_length = 0;
        _YTABLE_CONFIG_SETTING_INT_OPTIONAL(setting, YTABLE_CONFIG_MEMBER_DB_LENGTH, db_length, 0);
        const char * db_prefix;
        _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(setting, YTABLE_CONFIG_MEMBER_DB_PREFIX, db_prefix, NULL);

        yvar_t table_length_var = YVAR_INT32(table_length);
        yvar_t db_length_var = YVAR_INT32(db_length);
        ysize_t db_prefix_len = 0;
        yvar_t db_prefix_var = YVAR_EMPTY();
        if (db_prefix) {
            db_prefix_len = strlen(db_prefix);
            yvar_cstr_with_size(db_prefix_var, db_prefix, db_prefix_len);
        }
        if (db_length > 0 && db_prefix_len == 0) {
            YUKI_LOG_FATAL("hash db config is not right params");
            return yfalse;
        }
        yvar_t keys_arr[] = {YVAR_CSTR(YTABLE_CONFIG_MEMBER_TABLE_LENGTH), YVAR_CSTR(YTABLE_CONFIG_MEMBER_DB_LENGTH), YVAR_CSTR(YTABLE_CONFIG_MEMBER_DB_PREFIX)};
        yvar_t keys = YVAR_ARRAY(keys_arr);
        yvar_t values_arr[] = {table_length_var, db_length_var, db_prefix_var};
        yvar_t values = YVAR_ARRAY(values_arr);
        yvar_t params_var = YVAR_EMPTY();
        yvar_map(params_var, keys, values);

        ybool_t ret = yvar_pin(config->params, params_var);

        if (!ret) {
            YUKI_LOG_FATAL("cannot pin params");
            return yfalse;
        }
        config->hash_method = YTABLE_HASH_METHOD_KEY_HASH;
    } else {
        YUKI_LOG_FATAL("can not match hash method");
        return yfalse;
    }

    return ytrue;
}

static inline ybool_t _ytable_inited()
{
    return g_ytable_inited;
}

static inline void _ytable_set_last_error(ytable_t * ytable, ytable_error_t error)
{
    if (ytable) {
        ytable->last_error = error;
    }
}

static inline ybool_t _ytable_sql_is_valid_verb(ytable_verb_t verb)
{
    return verb > YTABLE_VERB_NULL && verb < YTABLE_VERB_MAX;
}

static inline const char * _ytable_sql_get_verb(ytable_verb_t verb)
{
    YUKI_ASSERT(_ytable_sql_is_valid_verb(verb) || YTABLE_VERB_NULL == verb);

    return g_ytable_sql_builder[verb].verb;
}

static inline void _ytable_set_active_connection(ytable_t * ytable, ytable_connection_t * conn)
{
    YUKI_ASSERT(ytable && conn);
    ytable->active_connection = conn;
}

static inline ytable_connection_t * _ytable_get_active_connection(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    return ytable->active_connection;
}

static ybool_t _ytable_sql_do_validate(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(_ytable_sql_is_valid_verb(ytable->verb));

    return g_ytable_sql_builder[ytable->verb].validator(ytable);
}

static ybool_t _ytable_sql_do_build(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(_ytable_sql_is_valid_verb(ytable->verb));

    return g_ytable_sql_builder[ytable->verb].builder(ytable);
}

static ybool_t _ytable_sql_do_result_parse(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(_ytable_sql_is_valid_verb(ytable->verb));

    return g_ytable_sql_builder[ytable->verb].parser(ytable, mysql_res, result);
}

static inline ybool_t _ytable_check_verb(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);

    if (YTABLE_VERB_NULL != ytable->verb) {
        YUKI_LOG_DEBUG("verb can only be set once. [verb: %s]", _ytable_sql_get_verb(ytable->verb));
        return yfalse;
    }

    return ytrue;
}

static ybool_t _ytable_sql_select_validator(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_SELECT);

    if (!ytable->fields || !yvar_is_array(*ytable->fields) || !yvar_count(*ytable->fields)) {
        YUKI_LOG_DEBUG("fields is invalid");
        return yfalse;
    }

    // FIXME: condition can be YVAR_BOOL(ytrue)
    if (!ytable->conditions || !yvar_is_array(*ytable->conditions) || !yvar_count(*ytable->conditions)) {
        YUKI_LOG_DEBUG("conditions is invalid");
        return yfalse;
    }

    return ytrue;
}

static ybool_t _ytable_sql_update_validator(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_UPDATE);

    if (!ytable->fields || !yvar_is_array(*ytable->fields) || !yvar_count(*ytable->fields)) {
        YUKI_LOG_DEBUG("fields is invalid");
        return yfalse;
    }

    // FIXME: condition can be YVAR_BOOL(ytrue)
    if (!ytable->conditions || !yvar_is_array(*ytable->conditions) || !yvar_count(*ytable->conditions)) {
        YUKI_LOG_DEBUG("conditions is invalid");
        return yfalse;
    }

    return ytrue;
}

static ybool_t _ytable_sql_insert_validator(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_INSERT);

    if (!ytable->fields || !yvar_is_map(*ytable->fields) || !yvar_count(*ytable->fields)) {
        YUKI_LOG_DEBUG("fields is invalid");
        return yfalse;
    }

    return ytrue;
}

static ybool_t _ytable_sql_delete_validator(const ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_DELETE);

    // FIXME: condition can be YVAR_BOOL(ytrue)
    if (!ytable->conditions || !yvar_is_array(*ytable->conditions) || !yvar_count(*ytable->conditions)) {
        YUKI_LOG_DEBUG("conditions is invalid");
        return yfalse;
    }

    return ytrue;
}

static inline ysize_t _ytable_sql_estimate_field_size(const yvar_t * field)
{
    YUKI_ASSERT(field);
    YUKI_ASSERT(yvar_like_string(*field));

    return yvar_cstr_strlen(*field);
}

static inline ysize_t _ytable_sql_estimate_value_size(const yvar_t * value)
{
    YUKI_ASSERT(value);

    if (yvar_like_int(*value)) {
        return _YTABLE_SQL_INT_MAXLEN;
    } else if (yvar_like_string(*value)) {
        // in the worst case, every char needs to be escaped
        return yvar_cstr_strlen(*value) * 2;
    } else {
        // TODO: support other types
        YUKI_ASSERT(yfalse);
        YUKI_LOG_FATAL("condition value can only be int and string");
        return 0;
    }
}



static ybool_t _ytable_sql_estimate_where(const ytable_t * ytable, ysize_t * result)
{
    YUKI_ASSERT(ytable && result);
    YUKI_ASSERT(ytable->conditions);

    ysize_t size = 0;

    if (yvar_is_array(*ytable->conditions)) {
        size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_WHERE);

        FOREACH_YVAR_ARRAY(*ytable->conditions, value) {
            if (!yvar_is_array(*value) || yvar_count(*value) != 3) {
                YUKI_LOG_FATAL("triple array must be array and have 3 elements");
                return yfalse;
            }

            yvar_t the_field = YVAR_EMPTY();
            yvar_t the_op = YVAR_EMPTY();
            yvar_t the_value = YVAR_EMPTY();

            yvar_array_get(*value, 0, the_field);
            yvar_array_get(*value, 1, the_op);
            yvar_array_get(*value, 2, the_value);

            // TODO: check field and op type
            YUKI_ASSERT(yvar_like_string(the_field));
            YUKI_ASSERT(yvar_like_string(the_op));

            size += _ytable_sql_estimate_field_size(&the_field) + yvar_cstr_strlen(the_op)
                + _ytable_sql_estimate_value_size(&the_value)
                + 2 /* for space */ + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_FIELD)
                + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_VALUE);

            // FIXME: currently, only support AND
            size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_AND);
        }
    }

    *result += size;
    return ytrue;
}


static ysize_t _ytable_sql_hash_table_length(ytable_table_config_t * config)
{
    YUKI_ASSERT(config);

    yint32_t table_length = 0;
    yvar_t table_val = YVAR_CSTR(YTABLE_CONFIG_MEMBER_TABLE_LENGTH);
    yvar_t val = YVAR_EMPTY();
    if (yvar_map_get(*config->params, table_val, val)) {
        yvar_get_int32(val,table_length);
    }

    return table_length;
}

static ysize_t _ytable_sql_hash_db_length(ytable_table_config_t * config)
{
    YUKI_ASSERT(config);

    yint32_t db_length = 0;
    yvar_t db_val = YVAR_CSTR(YTABLE_CONFIG_MEMBER_DB_LENGTH);
    yvar_t val = YVAR_EMPTY();

    if (yvar_map_get(*config->params, db_val, val)) {
        yvar_get_int32(val,db_length);
    }

    return db_length;
}

static ysize_t _ytable_sql_hash_db_prefix_length(ytable_table_config_t * config)
{
    YUKI_ASSERT(config);

    ysize_t db_prefix = 0;
    yvar_t db_val = YVAR_CSTR(YTABLE_CONFIG_MEMBER_DB_PREFIX);
    yvar_t val = YVAR_EMPTY();
    const yvar_t empty = YVAR_EMPTY();

    if (yvar_map_get(*config->params, db_val, val)) {
        if (yvar_equal(val, empty)) {
            return db_prefix;
        }
        db_prefix = yvar_cstr_strlen(val);
        db_prefix += 3;     //1 for dot ,2 for _YTABLE_SQL_QUOT_FIELD
    }
    return db_prefix;
}


static ybool_t _ytable_sql_estimate_table(const ytable_t * ytable, ysize_t * result)
{
    YUKI_ASSERT(ytable && result);

    YUKI_LOG_DEBUG("estimate table");

    ytable_table_config_t * config = &g_ytable_table_configs[ytable->ytable_index];
    ysize_t size = 0;
    // TODO: implement hash
    if (config->hash_method == YTABLE_HASH_METHOD_KEY_HASH) {
        size += _ytable_sql_hash_table_length(config)
              + _ytable_sql_hash_db_length(config)
              + _ytable_sql_hash_db_prefix_length(config);
    }

    // 1 is for space at the end and 2 for table _YTABLE_SQL_QUOT_FIELD
    //looks like `table`
    size += config->name_len + 3;

    *result += size;
    return ytrue;
}

static const char * _ytable_sql_do_build_value(const ytable_t * ytable, const yvar_t * yvar, char * buffer, ysize_t size)
{
    YUKI_ASSERT(ytable && yvar && buffer && size);

    // do real escaple on value
    if (yvar_like_string(*yvar)) {
        YUKI_ASSERT(size >= yvar_cstr_strlen(*yvar) * 2 + 1);
        mysql_real_escape_string(&_ytable_get_active_connection(ytable)->mysql,
            buffer, yvar_cstr_buffer(*yvar), yvar_cstr_strlen(*yvar));
    } else {
        ybool_t ret = yvar_get_str(*yvar, buffer, size);
        YUKI_ASSERT(ret);
    }

    return buffer;
}

static ybool_t _ytable_sql_do_build_where(const ytable_t * ytable, char * buffer, ysize_t size, ysize_t * offset)
{
    YUKI_ASSERT(ytable && buffer && size && offset);
    YUKI_ASSERT(ytable->conditions);

    if (yvar_is_array(*ytable->conditions)) {
        *offset += snprintf(buffer + *offset, size - *offset, "%s", _YTABLE_SQL_KEYWORD_WHERE);

        // estimate value buffer, very rough but very efficient.
        ysize_t value_size = size - *offset;
        char value_buffer[value_size];

        FOREACH_YVAR_ARRAY(*ytable->conditions, value) {
            YUKI_ASSERT(yvar_is_array(*value) && yvar_count(*value) == 3);

            yvar_t the_field = YVAR_EMPTY();
            yvar_t the_op = YVAR_EMPTY();
            yvar_t the_value = YVAR_EMPTY();
            yvar_array_get(*value, 0, the_field);
            yvar_array_get(*value, 1, the_op);
            yvar_array_get(*value, 2, the_value);

            // TODO: check field and op type
            YUKI_ASSERT(yvar_like_string(the_field));
            YUKI_ASSERT(yvar_like_string(the_op));

            // `field` op 'value'
            *offset += snprintf(buffer + *offset, size - *offset,
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD " %s "
                _YTABLE_SQL_QUOT_VALUE "%s" _YTABLE_SQL_QUOT_VALUE _YTABLE_SQL_KEYWORD_AND,
                yvar_cstr_buffer(the_field), yvar_cstr_buffer(the_op),
                _ytable_sql_do_build_value(ytable, &the_value, value_buffer, value_size));
        }

        // remove tailing AND
        *offset -= _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_AND);
    }

    return ytrue;
}

static inline ysize_t _ypower(ysize_t base, ysize_t expon)
{
    ysize_t result = 1;
    ysize_t i;
    for (i = 0; i < expon; i++) {
        result *= base;
    }
    return result;
}

#define YUKI_HASH_KEY_BUF_LEN  64
static ybool_t _ytable_sql_get_hash_key(yvar_t key, ysize_t key_len, ysize_t * hash_key)
{
    if (yvar_like_int(key)) {
        ysize_t hk2 = 0;
        yvar_get_uint64(key, hk2);
        *hash_key = hk2 % _ypower(10, key_len);
    } else if (yvar_like_string(key)) {
        char buf[YUKI_HASH_KEY_BUF_LEN] = {0};
        char hk[32] = {0};
        ysize_t len = yvar_cstr_strlen(key);
        snprintf(buf, YUKI_HASH_KEY_BUF_LEN, "%s", yvar_cstr_buffer(key));
        if (key_len > len) {
            key_len = len;
        }
        ysize_t i;
        for (i = 0; i < key_len; i++) {
            hk[i] = *(yvar_cstr_buffer(key) + len - key_len + i);
        }
        *hash_key = atol(hk);
    } else {
        YUKI_LOG_WARNING("hash key type not define");
        return yfalse;
    }

    return ytrue;
}



static ybool_t _ytable_sql_do_build_table(const ytable_t * ytable, char * buffer, ysize_t size, ysize_t * offset)
{
    YUKI_ASSERT(ytable && buffer && size && offset);

    ytable_table_config_t * config = &g_ytable_table_configs[ytable->ytable_index];
    static const yvar_t op_equal = YVAR_CSTR("=");
    yvar_t * table_data;

    // TODO: implement hash
    if (config->hash_method == YTABLE_HASH_METHOD_KEY_HASH) {
        switch (ytable->verb)
        {
        case YTABLE_VERB_SELECT:
        case YTABLE_VERB_UPDATE:
        case YTABLE_VERB_DELETE:
            //find hash key in conditions
            table_data = ytable->conditions;
            break;
        case YTABLE_VERB_INSERT:
            //find hash key in fields
            table_data = ytable->fields;
            break;
        default :
            YUKI_LOG_WARNING("error verb in table");
            return yfalse;
        }

        // TODO: find hash key in conditions
        FOREACH_YVAR_ARRAY(*table_data, value) {
            YUKI_ASSERT(yvar_is_array(*value) && yvar_count(*value) == 3);
            yvar_t the_field = YVAR_EMPTY();
            yvar_array_get(*value, 0, the_field);

            // TODO: check field type
            YUKI_ASSERT(yvar_like_string(the_field));

            // TODO: find hash key
            if (!strcmp(yvar_cstr_buffer(the_field),config->hash_key)) {

                yvar_t the_op = YVAR_EMPTY();
                yvar_t the_value = YVAR_EMPTY();

                yvar_array_get(*value, 1, the_op);
                yvar_array_get(*value, 2, the_value);

                // TODO: check op type
                YUKI_ASSERT(yvar_like_string(the_op));


                if (!yvar_equal(the_op, op_equal)) {
                    YUKI_LOG_WARNING("hash key not use equ op");
                    return yfalse;
                }

                ysize_t table_length = _ytable_sql_hash_table_length( config);
                ysize_t db_length = _ytable_sql_hash_db_length(config);
                ysize_t db_prefix_length = _ytable_sql_hash_db_prefix_length(config);
                ysize_t hash_key = 0;
                _ytable_sql_get_hash_key(the_value, table_length + db_length, &hash_key);
                if (db_prefix_length > 0) {
                    yvar_t db_val = YVAR_CSTR(YTABLE_CONFIG_MEMBER_DB_PREFIX);
                    yvar_t db_prefix;
                    yvar_map_get(*config->params, db_val, db_prefix);

                    if (db_length > 0) {
                        *offset += snprintf(buffer + *offset, size - *offset,
                                    _YTABLE_SQL_QUOT_FIELD "%s%ld" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_DOT ,      //db name
                                    yvar_cstr_buffer(db_prefix), hash_key/_ypower(10,table_length));
                    } else {
                        *offset += snprintf(buffer + *offset, size - *offset,
                                    _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_DOT ,      //db name
                                    yvar_cstr_buffer(db_prefix));
                    }
                }

                *offset += snprintf(buffer + *offset, size - *offset,
                _YTABLE_SQL_QUOT_FIELD "%s%ld" _YTABLE_SQL_QUOT_FIELD,                     //table name
                config->name,hash_key);
                break;
            }
        }
    } else {
        *offset += snprintf(buffer + *offset, size - *offset,
            _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD,
            config->name);
    }

    return ytrue;
}

static ybool_t _ytable_sql_select_builder(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_SELECT);
    YUKI_ASSERT(ytable->fields && ytable->conditions);

    if (yvar_like_string(ytable->sql)) {
        YUKI_LOG_DEBUG("sql was built. sql: %s", yvar_cstr_buffer(ytable->sql));
        return ytrue;
    }

    ysize_t size = 1; // space for tailing '\0'
    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_VERB_SELECT);

    // estimate buffer length for fields
    FOREACH_YVAR_ARRAY(*ytable->fields, field) {
        // FIXME: field can be an array with 2 elements
        if (!yvar_like_string(*field)) {
            YUKI_LOG_DEBUG("field can only be str/cstr or array with 2 element");
            return yfalse;
        }

        size += _ytable_sql_estimate_field_size(field) + _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA);
    }

    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_FROM);

    if (!_ytable_sql_estimate_table(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate table name size");
        return yfalse;
    }

    if (!_ytable_sql_estimate_where(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate where condition size");
        return yfalse;
    }

    char * buffer = ybuffer_simple_alloc(size);

    if (!buffer) {
        YUKI_LOG_WARNING("out of memory");
        return yfalse;
    }

    ysize_t offset = 0;
    offset += snprintf(buffer + offset, size - offset, "%s", _YTABLE_SQL_VERB_SELECT);

    // FIXME: field can be an array. assume it's a string right now.
    FOREACH_YVAR_ARRAY(*ytable->fields, field) {
        YUKI_ASSERT(yvar_like_string(*field));

        offset += snprintf(buffer + offset, size - offset,
            "%s" _YTABLE_SQL_COMMA,
            yvar_cstr_buffer(*field));
    }

    // remove tailing comma
    offset -= _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA);

    offset += snprintf(buffer + offset, size - offset, "%s", _YTABLE_SQL_KEYWORD_FROM);

    if (!_ytable_sql_do_build_table(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build table name");
        return yfalse;
    }

    if (!_ytable_sql_do_build_where(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build where condition");
        return yfalse;
    }

    buffer[offset] = '\0';

    YUKI_LOG_DEBUG("built sql: %s", buffer);
    yvar_cstr_with_size(ytable->sql, buffer, offset);
    return ytrue;
}

static ybool_t _ytable_sql_update_builder(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_UPDATE);
    YUKI_ASSERT(ytable->fields && ytable->conditions);
    YUKI_ASSERT(yvar_is_array(*ytable->fields));

    if (yvar_like_string(ytable->sql)) {
        YUKI_LOG_DEBUG("sql was built. sql: %s", yvar_cstr_buffer(ytable->sql));
        return ytrue;
    }

    ysize_t size = 1; // space for tailing '\0'
    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_VERB_UPDATE);

    if (!_ytable_sql_estimate_table(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate table name size");
        return yfalse;
    }

    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_SET);
    ysize_t old_size = size;

    // estimate buffer length for fields
    FOREACH_YVAR_ARRAY(*ytable->fields, value1) {
        if (!yvar_is_array(*value1) || yvar_count(*value1) != 3) {
            YUKI_LOG_DEBUG("field key can only be str/cstr");
            return yfalse;
        }

        yvar_t the_field = YVAR_EMPTY();
        yvar_t the_op = YVAR_EMPTY();
        yvar_t the_value = YVAR_EMPTY();
        yvar_array_get(*value1, 0, the_field);
        yvar_array_get(*value1, 1, the_op);
        yvar_array_get(*value1, 2, the_value);

        size += _ytable_sql_estimate_field_size(&the_field) * 2 /* may need 2 field if op is '+=' */
            + _ytable_sql_estimate_value_size(&the_value) + _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_VALUE)
            + yvar_cstr_strlen(the_op)
            + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_OP_EQ) + 4 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_FIELD);
    }

    // a rough but quick estimate on max value buffer.
    ysize_t value_size = size - old_size;

    if (!_ytable_sql_estimate_where(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate where condition size");
        return yfalse;
    }

    char * buffer = ybuffer_simple_alloc(size);

    if (!buffer) {
        YUKI_LOG_WARNING("out of memory");
        return yfalse;
    }

    ysize_t offset = 0;
    offset += snprintf(buffer + offset, size - offset, "%s", _YTABLE_SQL_VERB_UPDATE);

    if (!_ytable_sql_do_build_table(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build table name");
        return yfalse;
    }

    offset += snprintf(buffer + offset, size - offset, "%s", _YTABLE_SQL_KEYWORD_SET);

    static const yvar_t op_plus = YVAR_CSTR("+=");
    static const yvar_t op_minus = YVAR_CSTR("-=");
    static const yvar_t op_equal = YVAR_CSTR("=");
    char value_buffer[value_size];

    FOREACH_YVAR_ARRAY(*ytable->fields, value2) {
        // FIXME: support more types
        YUKI_ASSERT(yvar_is_array(*value2) && yvar_count(*value2));

        yvar_t the_field = YVAR_EMPTY();
        yvar_t the_op = YVAR_EMPTY();
        yvar_t the_value = YVAR_EMPTY();
        yvar_array_get(*value2, 0, the_field);
        yvar_array_get(*value2, 1, the_op);
        yvar_array_get(*value2, 2, the_value);

        if (yvar_equal(the_op, op_plus)) {
            // `field` = `field` + 'value'
            offset += snprintf(buffer + offset, size - offset,
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_OP_EQ
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_OP_PLUS
                _YTABLE_SQL_QUOT_VALUE "%s" _YTABLE_SQL_QUOT_VALUE _YTABLE_SQL_COMMA,
                yvar_cstr_buffer(the_field), yvar_cstr_buffer(the_field),
                _ytable_sql_do_build_value(ytable, &the_value, value_buffer, value_size));
        } else if (yvar_equal(the_op, op_minus)) {
            // `field` = `field` - 'value'
            offset += snprintf(buffer + offset, size - offset,
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_OP_EQ
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_OP_MINUS
                _YTABLE_SQL_QUOT_VALUE "%s" _YTABLE_SQL_QUOT_VALUE _YTABLE_SQL_COMMA,
                yvar_cstr_buffer(the_field), yvar_cstr_buffer(the_field),
                _ytable_sql_do_build_value(ytable, &the_value, value_buffer, value_size));
        } else if (yvar_equal(the_op, op_equal)) {
            // `field` op 'value'
            offset += snprintf(buffer + offset, size - offset,
                _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_OP_EQ
                _YTABLE_SQL_QUOT_VALUE "%s" _YTABLE_SQL_QUOT_VALUE _YTABLE_SQL_COMMA,
                yvar_cstr_buffer(the_field),
                _ytable_sql_do_build_value(ytable, &the_value, value_buffer, value_size));
        } else {
            YUKI_LOG_WARNING("unsupported op in UPDATE fields. [op: %s]", yvar_cstr_buffer(the_op));
            return yfalse;
        }

    }

    // remove tailing comma
    offset -= _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA);

    if (!_ytable_sql_do_build_where(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build where condition");
        return yfalse;
    }

    buffer[offset] = '\0';

    YUKI_LOG_DEBUG("built sql: %s", buffer);
    yvar_cstr_with_size(ytable->sql, buffer, offset);
    return ytrue;
}

static ybool_t _ytable_sql_insert_builder(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_INSERT);
    YUKI_ASSERT(ytable->fields);
    YUKI_ASSERT(yvar_is_map(*ytable->fields));

    if (yvar_like_string(ytable->sql)) {
        YUKI_LOG_DEBUG("sql was built. sql: %s", yvar_cstr_buffer(ytable->sql));
        return ytrue;
    }

    ysize_t size = 1; // space for tailing '\0'
    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_VERB_INSERT);

    if (!_ytable_sql_estimate_table(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate table name size");
        return yfalse;
    }

    size += 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_BRACKET_LEFT) + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_BRACKET_RIGHT)
        + _YTABLE_SQL_STRLEN(_YTABLE_SQL_KEYWORD_VALUES) + 1 /*space for a blank*/;
    ysize_t old_size = size;

    // estimate buffer length for fields
    FOREACH_YVAR_MAP(*ytable->fields, key1, value1) {
        if (!yvar_like_string(*key1)) {
            YUKI_LOG_DEBUG("field key can only be str/cstr");
            return yfalse;
        }

        if (!yvar_like_string(*value1) && !yvar_like_int(*value1)) {
            YUKI_LOG_DEBUG("field value can only be str/cstr/int");
            return yfalse;
        }

        size += _ytable_sql_estimate_field_size(key1) + _ytable_sql_estimate_value_size(value1)
            + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA)
            + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_VALUE)
            + 2 * _YTABLE_SQL_STRLEN(_YTABLE_SQL_QUOT_FIELD);
    }

    // a rough but quick estimate on max value buffer.
    ysize_t value_size = size - old_size;

    char * buffer = ybuffer_simple_alloc(size);

    if (!buffer) {
        YUKI_LOG_WARNING("out of memory");
        return yfalse;
    }

    ysize_t offset = 0;
    offset += snprintf(buffer + offset, size - offset, "%s", _YTABLE_SQL_VERB_INSERT);

    if (!_ytable_sql_do_build_table(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build table name");
        return yfalse;
    }

    offset += snprintf(buffer + offset, size - offset, " %s",
        _YTABLE_SQL_BRACKET_LEFT);

    // built key list
    FOREACH_YVAR_MAP(*ytable->fields, key2, value2) {
        YUKI_ASSERT(yvar_like_string(*key2));

        offset += snprintf(buffer + offset, size - offset,
            _YTABLE_SQL_QUOT_FIELD "%s" _YTABLE_SQL_QUOT_FIELD _YTABLE_SQL_COMMA,
            yvar_cstr_buffer(*key2));
    }

    // remove tailing comma
    offset -= _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA);

    offset += snprintf(buffer + offset, size - offset, "%s",
        _YTABLE_SQL_BRACKET_RIGHT _YTABLE_SQL_KEYWORD_VALUES _YTABLE_SQL_BRACKET_LEFT);

    char value_buffer[value_size];

    // built value list
    FOREACH_YVAR_MAP(*ytable->fields, key3, value3) {
        YUKI_ASSERT(yvar_like_string(*value3) || yvar_like_int(*value3));
        offset += snprintf(buffer + offset, size - offset,
            _YTABLE_SQL_QUOT_VALUE "%s" _YTABLE_SQL_QUOT_VALUE _YTABLE_SQL_COMMA,
            _ytable_sql_do_build_value(ytable, value3, value_buffer, value_size));
    }

    // remove tailing comma
    offset -= _YTABLE_SQL_STRLEN(_YTABLE_SQL_COMMA);

    offset += snprintf(buffer + offset, size - offset, "%s",
        _YTABLE_SQL_BRACKET_RIGHT);

    buffer[offset] = '\0';

    YUKI_LOG_DEBUG("built sql: %s", buffer);
    yvar_cstr_with_size(ytable->sql, buffer, offset);
    return ytrue;
}

static ybool_t _ytable_sql_delete_builder(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);
    YUKI_ASSERT(ytable->verb == YTABLE_VERB_DELETE);
    YUKI_ASSERT(ytable->conditions);

    if (yvar_like_string(ytable->sql)) {
        YUKI_LOG_DEBUG("sql was built. sql: %s", yvar_cstr_buffer(ytable->sql));
        return ytrue;
    }

    ysize_t size = 1; // space for tailing '\0'
    size += _YTABLE_SQL_STRLEN(_YTABLE_SQL_VERB_DELETE);

    if (!_ytable_sql_estimate_table(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate table name size");
        return yfalse;
    }

    if (!_ytable_sql_estimate_where(ytable, &size)) {
        YUKI_LOG_DEBUG("cannot estimate where condition size");
        return yfalse;
    }

    char * buffer = ybuffer_simple_alloc(size);

    if (!buffer) {
        YUKI_LOG_WARNING("out of memory");
        return yfalse;
    }

    ysize_t offset = 0;
    offset += snprintf(buffer + offset, size - offset, "%s",
        _YTABLE_SQL_VERB_DELETE);

    if (!_ytable_sql_do_build_table(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build table name");
        return yfalse;
    }

    if (!_ytable_sql_do_build_where(ytable, buffer, size, &offset)) {
        YUKI_LOG_FATAL("cannot build where condition");
        return yfalse;
    }

    buffer[offset] = '\0';

    YUKI_LOG_DEBUG("built sql: %s", buffer);
    yvar_cstr_with_size(ytable->sql, buffer, offset);
    return ytrue;
}

static ybool_t _ytable_sql_select_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result)
{
    // TODO: finish it
    YUKI_ASSERT(ytable && mysql_res && result);
    YUKI_ASSERT(ytable->fields && yvar_is_array(*ytable->fields));
    MYSQL_RES * res = (MYSQL_RES*)mysql_res;

    if (!ytable->affected_rows) {
        YUKI_LOG_DEBUG("empty result set");
        *result = &g_ytable_result_false;
        return ytrue;
    }

    ysize_t field_cnt = mysql_num_fields(res);

    if (!field_cnt) {
        YUKI_LOG_FATAL("field count is 0. what's wrong?");
        return yfalse;
    }

    yvar_t local_result[ytable->affected_rows * field_cnt];
    enum enum_field_types field_types[field_cnt];
    yuint64_t field_flags[field_cnt];
    yvar_t field_raw_key[field_cnt];
    MYSQL_ROW row;
    MYSQL_FIELD * field = NULL;
    uint64_t * lengths = NULL;
    ysize_t affected_rows = ytable->affected_rows;
    ysize_t cnt;
    ysize_t i;

    yint64_t temp_signed = 0;
    yuint64_t temp_unsigned = 0;
    ybool_t is_unsigned = yfalse;

    for (cnt = 0; (field = mysql_fetch_field(res)) != NULL; cnt++) {
        YUKI_ASSERT(cnt < field_cnt);

        field_types[cnt] = field->type;
        field_flags[cnt] = field->flags;
        yvar_cstr_with_size(field_raw_key[cnt], field->name, field->name_length);
    }

    yvar_t field_keys = YVAR_ARRAY_WITH_SIZE(field_raw_key, field_cnt);

    for (cnt = 0; (row = mysql_fetch_row(res)) != NULL; cnt++) {
        YUKI_ASSERT(field_cnt == mysql_num_fields(res));
        YUKI_ASSERT(cnt < affected_rows);
        lengths = mysql_fetch_lengths(res);

        for (i = 0; i < field_cnt; i++) {
            if (NULL == row[i]) {
                YUKI_LOG_DEBUG("got a NULL value");
                yvar_undefined(local_result[cnt * field_cnt + i]);
                continue;
            }

            is_unsigned = field_flags[i] & UNSIGNED_FLAG;

            if (IS_NUM(field_types[i])) {
                errno = 0;

                if (is_unsigned) {
                    temp_unsigned = strtoull(row[i], NULL, 10);
                } else {
                    temp_signed = strtoll(row[i], NULL, 10);
                }

                if (errno) {
                    YUKI_LOG_WARNING("cannot convert numeric field to int. [errno: %d]", errno);
                    return yfalse;
                }
            }

            switch (field_types[i]) {
                case MYSQL_TYPE_TINY:
                    if (is_unsigned) {
                        yvar_uint8(local_result[cnt * field_cnt + i], temp_unsigned);
                    } else {
                        yvar_int8(local_result[cnt * field_cnt + i], temp_signed);
                    }

                    break;
                case MYSQL_TYPE_SHORT:
                    if (is_unsigned) {
                        yvar_uint16(local_result[cnt * field_cnt + i], temp_unsigned);
                    } else {
                        yvar_int16(local_result[cnt * field_cnt + i], temp_signed);
                    }

                    break;
                case MYSQL_TYPE_LONG:
                case MYSQL_TYPE_INT24:
                    if (is_unsigned) {
                        yvar_uint32(local_result[cnt * field_cnt + i], temp_unsigned);
                    } else {
                        yvar_int32(local_result[cnt * field_cnt + i],temp_signed);
                    }

                    break;
                case MYSQL_TYPE_LONGLONG:
                    if (is_unsigned) {
                        yvar_uint64(local_result[cnt * field_cnt + i], temp_unsigned);
                    } else {
                        yvar_int64(local_result[cnt * field_cnt + i], temp_signed);
                    }

                    break;
                case MYSQL_TYPE_STRING:
                case MYSQL_TYPE_VAR_STRING:
                case MYSQL_TYPE_BLOB:
                // TODO: make special var type for timestamp and datetime
                case MYSQL_TYPE_TIMESTAMP:
                case MYSQL_TYPE_DATETIME:
                    yvar_cstr_with_size(local_result[cnt * field_cnt + i], row[i], lengths[i]);
                    break;
                case MYSQL_TYPE_NULL:
                    YUKI_LOG_DEBUG("NULL type value");
                    yvar_undefined(local_result[cnt * field_cnt + i]);

                    break;
                default:
                    YUKI_LOG_WARNING("unsupported type. [type: %lu]", field_types[i]);
                    yvar_undefined(local_result[cnt * field_cnt + i]);
            }
        }
    }

    yvar_t value_array[affected_rows];
    yvar_t map_array[affected_rows];

    for (i = 0; i < affected_rows; i++) {
        yvar_array_with_size(value_array[i], local_result + i * field_cnt, field_cnt);
        yvar_map(map_array[i], field_keys, value_array[i]);
    }

    yvar_t result_var = YVAR_ARRAY_WITH_SIZE(map_array, affected_rows);
    return yvar_clone(*result, result_var);
}

static ybool_t _ytable_sql_update_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result)
{
    YUKI_ASSERT(ytable && result);
    (void)mysql_res;

    if (ytable->affected_rows) {
        *result = &g_ytable_result_true;
    } else {
        *result = &g_ytable_result_false;
    }

    return ytrue;
}

static ybool_t _ytable_sql_insert_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result)
{
    YUKI_ASSERT(ytable && result);
    (void)mysql_res;

    *result = &g_ytable_result_true;
    return ytrue;
}

static ybool_t _ytable_sql_delete_result_parser(const ytable_t * ytable, ytable_mysql_res_t * mysql_res, yvar_t ** result)
{
    YUKI_ASSERT(ytable && result);
    (void)mysql_res;

    if (ytable->affected_rows) {
        *result = &g_ytable_result_true;
    } else {
        *result = &g_ytable_result_false;
    }

    return ytrue;
}

static ybool_t _ytable_build_sql(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);

    if (!_ytable_sql_is_valid_verb(ytable->verb)) {
        YUKI_LOG_DEBUG("verb is not set");
        return yfalse;
    }

    if (!_ytable_sql_do_validate(ytable)) {
        YUKI_LOG_DEBUG("invalid/incomplete ytable instance");
        return yfalse;
    }

    const char * verb = _ytable_sql_get_verb(ytable->verb);
    YUKI_LOG_TRACE("start to build sql for verb %s", verb);

    if (!_ytable_sql_do_build(ytable)) {
        YUKI_LOG_DEBUG("fail to build sql");
        return yfalse;
    }

    YUKI_LOG_DEBUG("finish building sql");
    return ytrue;
}

static ytable_connection_thread_data_t * _ytable_thread_get_connection()
{
    ytable_connection_thread_data_t * thread_data = pthread_getspecific(g_ytable_connection_thread_key);

    if (!thread_data) {
        YUKI_LOG_TRACE("creating connection thread data...");

        ybuffer_t * buffer = ybuffer_create_global(sizeof(ytable_connection_thread_data_t));

        if (!buffer) {
            YUKI_LOG_WARNING("out of memory");
            return NULL;
        }

        thread_data = ybuffer_smart_alloc(buffer, ytable_connection_thread_data_t);

        if (!thread_data) {
            YUKI_LOG_FATAL("cannot alloc memory for thread data");
            return NULL;
        }

        thread_data->size = g_ytable_connection_configs_count;

        ysize_t conn_size = sizeof(ytable_connection_t) * g_ytable_connection_configs_count;
        buffer = ybuffer_create_global(conn_size);

        if (!buffer) {
            YUKI_LOG_WARNING("out of memory");
            return NULL;
        }

        ytable_connection_t * connections = (ytable_connection_t*)ybuffer_alloc(buffer, conn_size);

        if (!connections) {
            YUKI_LOG_FATAL("cannot alloc memory for connections");
            return NULL;
        }

        memset(connections, 0, conn_size);

        ysize_t cnt = 0;
        for (; cnt < thread_data->size; cnt++) {
            if (!mysql_init(&connections->mysql)) {
                YUKI_LOG_FATAL("fail to init mysql struct");
                return NULL;
            }

            connections->connected = yfalse;
        }

        thread_data->connections = connections;

        pthread_setspecific(g_ytable_connection_thread_key, thread_data);
    }

    return thread_data;
}

static ytable_connection_t * _ytable_fetch_db_connection(ytable_t * ytable)
{
    YUKI_ASSERT(ytable);

    ytable_connection_thread_data_t * thread_data = _ytable_thread_get_connection();

    if (!thread_data) {
        YUKI_LOG_FATAL("cannot get connection thread data");
        return NULL;
    }

    // FIXME: validate ytable index
    ytable_table_config_t * config = g_ytable_table_configs + ytable->ytable_index;
    yuint32_t connection_index;

    // FIXME: support array
    if (!yvar_get_uint32(*config->connection_index, connection_index)) {
        YUKI_LOG_WARNING("invalid connection_index");
        return NULL;
    }

    YUKI_ASSERT(connection_index < thread_data->size);

    ytable_connection_t * conn = thread_data->connections + connection_index;

    if (conn->connected) {
        if (mysql_ping(&conn->mysql)) {
            YUKI_LOG_TRACE("mysql connection is gone.");
            mysql_close(&conn->mysql);

            if (!mysql_init(&conn->mysql)) {
                YUKI_LOG_FATAL("fail to init mysql struct");
                return NULL;
            }

            conn->connected = yfalse;
        }
    }

    if (!conn->connected) {
        ytable_connection_config_t * conn_config = g_ytable_connection_configs + connection_index;

        if (!mysql_real_connect(&conn->mysql, conn_config->host, conn_config->user, conn_config->password, conn_config->database, conn_config->port, NULL, 0)) {
            YUKI_LOG_FATAL("cannot connect to mysql. [err: %s] [host: %s] [user: %s] [database: %s] [port: %lu]", mysql_error(&conn->mysql),
                conn_config->host, conn_config->user, conn_config->database, conn_config->port);
            return NULL;
        }

        if (conn_config->character_set) {
            if (mysql_set_character_set(&conn->mysql, conn_config->character_set)) {
                YUKI_LOG_FATAL("cannot set character set. [err: %s]", mysql_error(&conn->mysql));
                mysql_close(&conn->mysql);
                return NULL;
            }

            YUKI_LOG_TRACE("character set is set to '%s'", conn_config->character_set);
        }

        conn->connected = ytrue;
    }

    YUKI_LOG_TRACE("mysql connection is ready. [thread_id: %lu]", mysql_thread_id(&conn->mysql));
    return conn;
}

static ybool_t _ytable_execute(ytable_t * ytable, ytable_connection_t * conn)
{
    YUKI_ASSERT(conn && ytable);

    if (mysql_real_query(&conn->mysql, yvar_cstr_buffer(ytable->sql), yvar_cstr_strlen(ytable->sql))) {
        YUKI_LOG_WARNING("fail to execute query. [error: %s]", mysql_error(&conn->mysql));
        return yfalse;
    }

    return ytrue;
}

static void _ytable_connection_thread_clean_up(void * thread_data)
{
    if (!thread_data) {
        YUKI_LOG_TRACE("no thread data needs to be cleaned up");
        return;
    }

    ytable_connection_thread_data_t * data = (ytable_connection_thread_data_t*)thread_data;
    ysize_t index;

    for (index = 0; index < data->size; index++) {
        if (data->connections[index].connected) {
            mysql_close(&data->connections[index].mysql);
        }
    }
}

static ybool_t _ytable_fetch_internal(ytable_t * ytable, yvar_t ** result, yint32_t expected_rows)
{
    if (!ytable || !result) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return yfalse;
    }

    ytable_t local_table = *ytable;

    if (expected_rows >= 0) {
        // for fetch one, only allow to get up to 2 rows
        local_table.limit = expected_rows + 1;
    }

    ytable_connection_t * conn = _ytable_fetch_db_connection(&local_table);

    if (!conn) {
        YUKI_LOG_FATAL("cannot fetch a valid connection");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONNECTION);
        return yfalse;
    }

    _ytable_set_active_connection(&local_table, conn);

    if (!_ytable_build_sql(&local_table)) {
        YUKI_LOG_WARNING("unable to build sql");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_BUILD_SQL);
        return yfalse;
    }

    if (!_ytable_execute(&local_table, conn)) {
        YUKI_LOG_FATAL("fail to execute sql");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONNECTION);
        return yfalse;
    }

    ytable_mysql_res_t * mysql_res = (ytable_mysql_res_t*)mysql_store_result(&conn->mysql);

    if (!mysql_res) {
        if (mysql_field_count(&conn->mysql) == 0) {
            local_table.affected_rows = mysql_affected_rows(&conn->mysql);
        } else {
            YUKI_LOG_WARNING("cannot store query result. [error: %s]", mysql_error(&conn->mysql));
            _ytable_set_last_error(ytable, YTABLE_ERROR_CONNECTION);
            return yfalse;
        }
    } else {
        local_table.affected_rows = mysql_affected_rows(&conn->mysql);
    }

    if (expected_rows >= 0) {
        // must be affected one row
        if (local_table.affected_rows != (ysize_t)expected_rows) {
            YUKI_LOG_WARNING("affected row is not %ld. [row: %lu]", expected_rows, local_table.affected_rows);
            mysql_free_result((MYSQL_RES*)mysql_res);
            _ytable_set_last_error(ytable, YTABLE_ERROR_NOT_EXPECTED_RESULT);
            return yfalse;
        }
    }

    // restore limit
    local_table.limit = ytable->limit;
    *ytable = local_table;

    ybool_t ret = _ytable_sql_do_result_parse(ytable, mysql_res, result);

    if (ret) {
        _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    } else {
        YUKI_LOG_WARNING("cannot parse result");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_PARSE_RESULT);
    }

    if (mysql_res) {
        mysql_free_result((MYSQL_RES*)mysql_res);
    }

    return ret;
}

static ybool_t _ytable_init_connection(config_t * config)
{
    YUKI_ASSERT(config);

    config_setting_t * setting = config_lookup(config, YTABLE_CONFIG_PATH_CONNECTIONS);

    if (!setting) {
        YUKI_LOG_FATAL("cannot get '%s' in config file", YTABLE_CONFIG_PATH_CONNECTIONS);
        return yfalse;
    }

    YUKI_LOG_DEBUG("reading %s", YTABLE_CONFIG_PATH_CONNECTIONS);

    if (CONFIG_TYPE_LIST != config_setting_type(setting)) {
        YUKI_LOG_FATAL("'%s' must be a list in config", YTABLE_CONFIG_PATH_CONNECTIONS);
        return yfalse;
    }

    ysize_t length = config_setting_length(setting);

    if (!length) {
        YUKI_LOG_FATAL("'%s' must have at least 1 element", YTABLE_CONFIG_PATH_CONNECTIONS);
        return yfalse;
    }

    ybuffer_t * buffer = ybuffer_create_global(sizeof(ytable_connection_config_t) * length);

    if (!buffer) {
        YUKI_LOG_FATAL("cannot create global buffer for connection configs");
        return yfalse;
    }

    g_ytable_connection_configs = ybuffer_alloc(buffer, sizeof(ytable_connection_config_t) * length);
    memset(g_ytable_connection_configs, 0, sizeof(ytable_connection_config_t) * length);
    g_ytable_connection_configs_count = length;

    if (!g_ytable_connection_configs) {
        YUKI_LOG_FATAL("cannot allocate memory for connection configs");
        return yfalse;
    }

    ysize_t index;
    for (index = 0; index < length; index++) {
        ytable_connection_config_t * cur = g_ytable_connection_configs + index;
        config_setting_t * conn = config_setting_get_elem(setting, index);
        YUKI_ASSERT(conn);

        if (CONFIG_TYPE_GROUP != config_setting_type(conn)) {
            YUKI_LOG_FATAL("'%s' element must be groups", YTABLE_CONFIG_PATH_CONNECTIONS);
            return yfalse;
        }

        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_NAME, cur->name);
        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_HOST, cur->host);
        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_USER, cur->user);
        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_PASSWORD, cur->password);
        _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(conn, YTABLE_CONFIG_MEMBER_DATABASE, cur->database, NULL);
        _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(conn, YTABLE_CONFIG_MEMBER_CHARACTER_SET, cur->character_set, NULL);
        _YTABLE_CONFIG_SETTING_INT_OPTIONAL(conn, YTABLE_CONFIG_MEMBER_PORT, cur->port, YTABLE_CONFIG_DEFAULT_PORT);
    }

    // estimate how many strings need to be copied
    ysize_t size = 0;

    for (index = 0; index < length; index++) {
        ytable_connection_config_t * cur = g_ytable_connection_configs + index;
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->name);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->host);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->user);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->password);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->database);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->character_set);
    }

    ybuffer_t * string_buffer = ybuffer_create_global(size);

    if (!string_buffer) {
        YUKI_LOG_FATAL("cannot create global buffer for connection strings");
        return yfalse;
    }

    for (index = 0; index < length; index++) {
        ytable_connection_config_t * cur = g_ytable_connection_configs + index;
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->name);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->host);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->user);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->password);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->database);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->character_set);
    }

    return ytrue;
}

static ybool_t _ytable_init_table(config_t * config)
{
    YUKI_ASSERT(config);

    config_setting_t * setting = config_lookup(config, YTABLE_CONFIG_PATH_TABLES);

    if (!setting) {
        YUKI_LOG_FATAL("cannot get '%s' in config file", YTABLE_CONFIG_PATH_TABLES);
        return yfalse;
    }

    YUKI_LOG_DEBUG("reading %s", YTABLE_CONFIG_PATH_TABLES);

    if (CONFIG_TYPE_LIST != config_setting_type(setting)) {
        YUKI_LOG_FATAL("'%s' must be a list in config", YTABLE_CONFIG_PATH_TABLES);
        return yfalse;
    }

    ysize_t length = config_setting_length(setting);

    if (!length) {
        YUKI_LOG_FATAL("'%s' must have at least 1 element", YTABLE_CONFIG_PATH_TABLES);
        return yfalse;
    }

    ybuffer_t * buffer = ybuffer_create_global(sizeof(ytable_table_config_t) * length);

    if (!buffer) {
        YUKI_LOG_FATAL("cannot create global buffer for table configs");
        return yfalse;
    }

    g_ytable_table_configs = ybuffer_alloc(buffer, sizeof(ytable_table_config_t) * length);
    memset(g_ytable_table_configs, 0, sizeof(ytable_table_config_t) * length);
    g_ytable_table_configs_count = length;

    if (!g_ytable_table_configs) {
        YUKI_LOG_FATAL("cannot allocate memory for table configs");
        return yfalse;
    }

    ysize_t index;
    for (index = 0; index < length; index++) {
        ytable_table_config_t * cur = g_ytable_table_configs + index;
        config_setting_t * conn = config_setting_get_elem(setting, index);
        YUKI_ASSERT(conn);

        if (CONFIG_TYPE_GROUP != config_setting_type(conn)) {
            YUKI_LOG_FATAL("'%s' element must be groups", YTABLE_CONFIG_PATH_TABLES);
            return yfalse;
        }

        const char * connection;
        const char * hash_method;

        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_NAME, cur->name);
        _YTABLE_CONFIG_SETTING_STRING(conn, YTABLE_CONFIG_MEMBER_CONNECTION, connection);
        _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(conn, YTABLE_CONFIG_MEMBER_HASH_KEY, cur->hash_key, NULL);
        _YTABLE_CONFIG_SETTING_STRING_OPTIONAL(conn, YTABLE_CONFIG_MEMBER_HASH_METHOD, hash_method, NULL);


        config_setting_t * conn2 = config_setting_get_member(conn, YTABLE_CONFIG_MEMBER_PARAMS);
        if (conn2) {
            if (!_ytable_config_set_hash_method(cur, hash_method, conn2)) {
                YUKI_LOG_FATAL("unknown hash_method %s", hash_method);
                return yfalse;
            }
        }

        cur->name_len = strlen(cur->name);

        ysize_t pos;
        for (pos = 0; pos < g_ytable_connection_configs_count; pos++) {
            if (!strcmp(connection, g_ytable_connection_configs[pos].name)) {
                yvar_t conn_index = YVAR_UINT64(pos);
                yvar_pin(cur->connection_index, conn_index);
                break;
            }
        }

        if (pos == g_ytable_connection_configs_count) {
            YUKI_LOG_FATAL("unknown connection '%s' in table '%s'",
                connection, cur->name);
            return yfalse;
        }
    }

    // estimate how many strings need to be copied
    ysize_t size = 0;

    for (index = 0; index < length; index++) {
        ytable_table_config_t * cur = g_ytable_table_configs + index;
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->name);
        _YTABLE_CONFIG_ESTIMATE_STRING(size, cur->hash_key);
    }

    ybuffer_t * string_buffer = ybuffer_create_global(size);

    if (!string_buffer) {
        YUKI_LOG_FATAL("cannot create global buffer for table strings");
        return yfalse;
    }

    for (index = 0; index < length; index++) {
        ytable_table_config_t * cur = g_ytable_table_configs + index;
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->name);
        _YTABLE_CONFIG_COPY_STRING(string_buffer, cur->hash_key);
    }

    return ytrue;
}

ybool_t _ytable_init(config_t * config)
{
    if (_ytable_inited()) {
        YUKI_LOG_DEBUG("ytable is inited");
        return ytrue;
    }

    YUKI_ASSERT(config);

    yvar_set_option(g_ytable_result_true, YVAR_OPTION_READONLY);
    yvar_set_option(g_ytable_result_false, YVAR_OPTION_READONLY);

    if (!_ytable_init_connection(config)) {
        YUKI_LOG_FATAL("cannot init connection config for ytable");
        return yfalse;
    }

    if (!_ytable_init_table(config)) {
        YUKI_LOG_FATAL("cannot init table config for ytable");
        return yfalse;
    }

    // init thread key
    // TODO: use pthread_once
    if (pthread_key_create(&g_ytable_connection_thread_key, &_ytable_connection_thread_clean_up)) {
        YUKI_LOG_FATAL("cannot create thread key for ytable. [err: %d]", errno);
        return yfalse;
    }

    g_ytable_inited = ytrue;
    return ytrue;
}

void _ytable_clean_up()
{
    // do nothing
}

void _ytable_shutdown()
{
    // FIXME: once ytable is shut down, it cannot be turned on with different config.
    // FIXME: thread data cannot be updated automatically after init ytable again.
    g_ytable_table_configs = NULL;
    g_ytable_table_configs_count = 0;
    g_ytable_connection_configs = NULL;
    g_ytable_connection_configs_count = 0;
    g_ytable_inited = yfalse;
}

ytable_t * ytable_instance(const char * table_name)
{
    if (!table_name) {
        YUKI_LOG_FATAL("invalid param");
        return NULL;
    }

    if (!_ytable_inited()) {
        YUKI_LOG_FATAL("use ytable before init it");
        return NULL;
    }

    ysize_t index;
    for (index = 0; index < g_ytable_table_configs_count; index++) {
        if (!strcmp(table_name, g_ytable_table_configs[index].name)) {
            YUKI_LOG_DEBUG("table is found. [name: %s] [index: %lu]", table_name, index);

            ybuffer_t * buffer = ybuffer_create_global(sizeof(ytable_t));

            if (!buffer) {
                YUKI_LOG_WARNING("out of memory");
                return NULL;
            }

            ytable_t * ytable = ybuffer_smart_alloc(buffer, ytable_t);

            if (!ytable) {
                YUKI_LOG_WARNING("out of memory");
                return NULL;
            }

            ytable->ytable_index = index;

            return ytable_reset(ytable);
        }
    }

    YUKI_LOG_WARNING("cannot find table '%s'", table_name);
    return NULL;
}

ytable_t * ytable_reset(ytable_t * ytable)
{
    ysize_t index = ytable->ytable_index;

    memset(ytable, 0, sizeof(ytable_t));
    ytable->ytable_index = index;
    ytable->limit = YTABLE_DEFAULT_LIMIT;
    ytable->offset = YTABLE_DEFAULT_OFFSET;

    return ytable;
}

ytable_t * _ytable_select(ytable_t * ytable, const yvar_t * fields)
{
    if (!ytable || !fields) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!yvar_is_array(*fields)) {
        YUKI_LOG_DEBUG("fields must be array");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_FIELD);
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        return ytable;
    }

    // TODO: validate fields

    ytable->verb = YTABLE_VERB_SELECT;

    if (!yvar_clone(ytable->fields, *fields)) {
        YUKI_LOG_FATAL("cannot clone field");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_insert(ytable_t * ytable, const yvar_t * values)
{
    if (!ytable || !values) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!yvar_is_map(*values)) {
        YUKI_LOG_DEBUG("values must be map");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONFLICTED_VERB);
        return ytable;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_INSERT;

    if (!yvar_clone(ytable->fields, *values)) {
        YUKI_LOG_FATAL("cannot clone field");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_insert_using_map_kv(ytable_t * ytable, yvar_map_kv_t values, ysize_t size)
{
    if (!ytable || !values) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONFLICTED_VERB);
        return ytable;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_INSERT;

    if (!yvar_map_clone(ytable->fields, values, size)) {
        YUKI_LOG_FATAL("cannot clone value fields");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}


ytable_t * _ytable_update(ytable_t * ytable, const yvar_t * values)
{
    if (!ytable || !values) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!yvar_is_array(*values)) {
        YUKI_LOG_DEBUG("values must be map");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONFLICTED_VERB);
        return ytable;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_UPDATE;

    if (!yvar_clone(ytable->fields, *values)) {
        YUKI_LOG_FATAL("cannot clone field");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_update_using_triple_array(ytable_t * ytable, yvar_triple_array_t values, ysize_t size)
{
    if (!ytable || !values || !size) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONFLICTED_VERB);
        return ytable;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_UPDATE;

    yvar_t fields[size];
    ysize_t index;

    for (index = 0; index < size; index++) {
        yvar_array_with_size(fields[index], values[size], size);
    }

    yvar_t fields_var = YVAR_ARRAY_WITH_SIZE(fields, size);

    if (yvar_clone(ytable->fields, fields_var)) {
        YUKI_LOG_FATAL("cannot clone field");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_delete(ytable_t * ytable)
{
    if (!ytable) {
        YUKI_LOG_FATAL("invalid param");
        // no need to set error
        return ytable;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CONFLICTED_VERB);
        return ytable;
    }

    ytable->verb = YTABLE_VERB_DELETE;

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_where(ytable_t * ytable, const yvar_t * conditions)
{
    if (!ytable || !conditions) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_sql_is_valid_verb(ytable->verb)) {
        YUKI_LOG_FATAL("verb must be set before using where");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_VERB);
        return ytable;
    }

    if (YTABLE_VERB_INSERT == ytable->verb) {
        YUKI_LOG_DEBUG("%s verb does not support where condition",
            _ytable_sql_get_verb(ytable->verb));
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_VERB);
        return ytable;
    }

    // TODO: support multiple where conditions
    if (ytable->conditions) {
        YUKI_LOG_DEBUG("only one condition can be used currently");
        _ytable_set_last_error(ytable, YTABLE_ERROR_NOT_IMPLEMENTED);
        return ytable;
    }

    // TODO: check conditions

    if (!yvar_clone(ytable->conditions, *conditions)) {
        YUKI_LOG_FATAL("cannot clone condition");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ytable_t * _ytable_where_using_triple_array(ytable_t * ytable, yvar_triple_array_t conditions, ysize_t size)
{
    if (!ytable || !conditions || !size) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return ytable;
    }

    if (!_ytable_sql_is_valid_verb(ytable->verb)) {
        YUKI_LOG_FATAL("verb must be set before using where");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_VERB);
        return ytable;
    }

    if (YTABLE_VERB_INSERT == ytable->verb) {
        YUKI_LOG_DEBUG("%s verb does not support where condition",
            _ytable_sql_get_verb(ytable->verb));
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_VERB);
        return ytable;
    }

    // TODO: support multiple where conditions
    if (ytable->conditions) {
        YUKI_LOG_DEBUG("only one condition can be used currently");
        _ytable_set_last_error(ytable, YTABLE_ERROR_NOT_IMPLEMENTED);
        return ytable;
    }

    // TODO: check conditions

    yvar_t raw_cond[size];
    ysize_t index;

    for (index = 0; index < size; index++) {
        yvar_array_with_size(raw_cond[index], conditions[index], size);
    }

    yvar_t cond = YVAR_ARRAY_WITH_SIZE(raw_cond, size);

    if (!yvar_clone(ytable->conditions, cond)) {
        YUKI_LOG_FATAL("cannot clone condition");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_CLONE_VAR);
        return ytable;
    }

    _ytable_set_last_error(ytable, YTABLE_ERROR_SUCCESS);
    return ytable;
}

ybool_t _ytable_fetch_all(ytable_t * ytable, yvar_t ** result)
{
    return _ytable_fetch_internal(ytable, result, -1);
}

ybool_t _ytable_fetch_one(ytable_t * ytable, yvar_t ** result)
{
    return _ytable_fetch_internal(ytable, result, 1);
}

ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id)
{
    if (!ytable || !insert_id) {
        YUKI_LOG_FATAL("invalid param");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_PARAM);
        return yfalse;
    }

    ytable_connection_t * conn = _ytable_get_active_connection(ytable);

    if (!conn) {
        YUKI_LOG_WARNING("fetch insert id before fetch result");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_FETCH_INSERT_ID);
        return yfalse;
    }

    if (YTABLE_VERB_INSERT != ytable->verb) {
        YUKI_LOG_WARNING("don't try to fetch insert id on verb other than INSERT");
        _ytable_set_last_error(ytable, YTABLE_ERROR_INVALID_VERB);
        return yfalse;
    }

    yuint64_t value = mysql_insert_id(&conn->mysql);

    if (!value) {
        YUKI_LOG_WARNING("cannot fetch insert id");
        _ytable_set_last_error(ytable, YTABLE_ERROR_CANNOT_FETCH_INSERT_ID);
        return yfalse;
    }

    YUKI_LOG_DEBUG("fetched insert id: %lu", value);
    yvar_uint64(*insert_id, value);
    return ytrue;
}

ytable_error_t ytable_last_error(const ytable_t * ytable)
{
    if (!ytable) {
        return YTABLE_ERROR_UNKNOWN;
    }

    return ytable->last_error;
}

