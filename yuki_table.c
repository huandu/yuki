#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <pthread.h>
#include <mysql.h>
#include <assert.h>

#include "yuki.h"

typedef struct _ytable_connection_t {
    MYSQL mysql;
    ybool_t connected;
} ytable_connection_t;

static ytable_config_t *g_ytable_configs = NULL;
static ytable_db_config_t *g_ytable_db_configs = NULL;
static yuint32_t g_ytable_db_configs_count = 0;

static ybool_t g_ytable_inited = yfalse;
static pthread_key_t g_ytable_connection_thread_key;

static inline ybool_t _ytable_inited()
{
    return g_ytable_inited;
}

static inline ybool_t _ytable_check_verb(const ytable_t * ytable)
{
    if (YTABLE_VERB_NULL != ytable->verb) {
        YUKI_LOG_DEBUG("only one of these verbs, select, insert, update and delete, can be used.");
        return yfalse;
    }

    return ytrue;
}

static ybool_t _ytable_build_sql(ytable_t * ytable)
{
    YUKI_LOG_FATAL("not implemented");
    return yfalse;
}

static ytable_connection_thread_data_t * _ytable_thread_get_connection()
{
    ytable_connection_thread_data_t * thread_data = pthread_getspecific(g_ytable_connection_thread_key);

    if (!thread_data) {
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

        thread_data->size = g_ytable_db_configs_count;

        ysize_t conn_size = sizeof(ytable_connection_t) * g_ytable_db_configs_count;
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

        memset(connections, conn_size, 0);

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
    ytable_connection_thread_data_t * thread_data = _ytable_thread_get_connection();

    if (!thread_data) {
        YUKI_LOG_FATAL("cannot get connection thread data");
        return NULL;
    }

    // TODO: validate ytable index
    ytable_config_t * config = g_ytable_configs + ytable->ytable_index;
    yuint32_t db_index;

    // FIXME: support array
    if (!yvar_get_uint32(*config->db_index, db_index)) {
        YUKI_LOG_WARNING("invalid db_index");
        return NULL;
    }

    YUKI_ASSERT(db_index < thread_data->size);

    ytable_connection_t * conn = thread_data->connections + db_index;

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
        ytable_db_config_t * db_config = g_ytable_db_configs + db_index;

        if (db_config->character_set) {
            mysql_options(&conn->mysql, MYSQL_SET_CHARSET_NAME, db_config->character_set);
            YUKI_LOG_TRACE("character set is set to '%s'", db_config->character_set);
        }

        if (!mysql_real_connect(&conn->mysql, db_config->host, db_config->user, db_config->password, db_config->database, db_config->port, NULL, 0)) {
            YUKI_LOG_FATAL("cannot connect to mysql. [err: %s] [host: %s] [user: %s] [database: %s] [port: %lu]", mysql_error(&conn->mysql),
                db_config->host, db_config->user, db_config->database, db_config->port);
            return NULL;
        }
    }

    YUKI_LOG_TRACE("mysql connection is ready. [thread_id: %lu]", mysql_thread_id(&conn->mysql));
    return conn;
}

static ybool_t _ytable_execute_one(ytable_connection_t * conn, ytable_t * ytable, yvar_t ** result)
{
    YUKI_LOG_FATAL("not implemented");
    return yfalse;
}

static void _ytable_connection_thread_clean_up(void * thread_data)
{
    // TODO: implement it
    YUKI_LOG_FATAL("not implemented");
}

ybool_t _ytable_init()
{
    if (_ytable_inited()) {
        YUKI_LOG_DEBUG("ytable is inited");
        return ytrue;
    }

    // TODO: remove test code
    g_ytable_configs = ybuffer_simple_alloc(sizeof(ytable_config_t) * 2);
    memset(g_ytable_configs, sizeof(ytable_config_t) * 2, 0);
    g_ytable_configs[0].table_name = "mytest";
    g_ytable_configs[0].hash_key = "uid";
    yvar_t db_index = YVAR_UINT32(0);
    yvar_pin(g_ytable_configs[0].db_index, db_index);
    g_ytable_configs[0].hash_method = YTABLE_HASH_METHOD_DEFAULT;

    g_ytable_db_configs = ybuffer_simple_alloc(sizeof(ytable_db_config_t));
    g_ytable_db_configs[0].db_name = "162";
    g_ytable_db_configs[0].host = "127.0.0.1";
    g_ytable_db_configs[0].user = "test";
    g_ytable_db_configs[0].password = "test";
    g_ytable_db_configs[0].database = "test";
    g_ytable_db_configs[0].character_set = "utf8";
    g_ytable_db_configs[0].port = 3306;
    g_ytable_db_configs_count = 1;

    // init thread key
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
    g_ytable_configs = NULL;
    g_ytable_inited = yfalse;
}

ytable_t * ytable_instance(const char * table_name)
{
    if (!_ytable_inited()) {
        YUKI_LOG_FATAL("use ytable before init it");
        return NULL;
    }

    // TODO: change test code

    return NULL;
}

ybool_t _ytable_select(ytable_t * ytable, const yvar_t * fields)
{
    if (!ytable || !fields) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!yvar_is_array(*fields)) {
        YUKI_LOG_DEBUG("fields must be array");
        return yfalse;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        return yfalse;
    }

    // TODO: validate fields

    ytable->verb = YTABLE_VERB_SELECT;

    return yvar_clone(ytable->fields, *fields);
}

ybool_t _ytable_insert(ytable_t * ytable, const yvar_t * values)
{
    if (!ytable || !values) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!yvar_is_array(*values)) {
        YUKI_LOG_DEBUG("values must be array");
        return yfalse;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        return yfalse;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_INSERT;

    return yvar_clone(ytable->fields, *values);
}

ybool_t _ytable_update(ytable_t * ytable, const yvar_t * values)
{
    if (!ytable || !values) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!yvar_is_array(*values)) {
        YUKI_LOG_DEBUG("values must be array");
        return yfalse;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        return yfalse;
    }

    // TODO: validate values

    ytable->verb = YTABLE_VERB_UPDATE;

    return yvar_clone(ytable->fields, *values);
}

ybool_t _ytable_delete(ytable_t * ytable)
{
    if (!ytable) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!_ytable_check_verb(ytable)) {
        YUKI_LOG_DEBUG("verb is set before");
        return yfalse;
    }

    ytable->verb = YTABLE_VERB_DELETE;

    return ytrue;
}

ybool_t _ytable_where(ytable_t * ytable, const yvar_t * conditions)
{
    if (!ytable || !conditions) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (YTABLE_VERB_NULL == ytable->verb) {
        YUKI_LOG_FATAL("verb must be set before using where");
        return yfalse;
    }

    // TODO: support multiple where conditions
    if (ytable->conditions) {
        YUKI_LOG_DEBUG("only one condition can be used currently");
        return yfalse;
    }

    // TODO: check conditions

    return yvar_clone(ytable->conditions, *conditions);
}

ybool_t _ytable_fetch_one(ytable_t * ytable, yvar_t ** result)
{
    if (!ytable || !result) {
        YUKI_LOG_FATAL("invalid param");
        return yfalse;
    }

    if (!_ytable_build_sql(ytable)) {
        YUKI_LOG_WARNING("unable to build sql");
        return yfalse;
    }

    ytable_connection_t * conn = _ytable_fetch_db_connection(ytable);

    if (!conn) {
        YUKI_LOG_FATAL("cannot fetch a valid connection");
        return yfalse;
    }

    if (!_ytable_execute_one(conn, ytable, result)) {
        YUKI_LOG_FATAL("fail to execute sql");
        return yfalse;
    }

    return ytrue;
}

ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id)
{
    // TODO: implement it
    YUKI_LOG_FATAL("not implemented");
    return yfalse;
}

