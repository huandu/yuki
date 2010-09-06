#ifndef _YUKI_TABLE_H_
#define _YUKI_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YTABLE_DEFAULT_LIMIT ((yint32_t)-1)
#define YTABLE_DEFAULT_OFFSET ((yint32_t)-1)

#define _YTABLE_SQL_STRLEN(s) (sizeof((s)) - 1)
#define _YTABLE_SQL_VERB_SELECT "SELECT "
#define _YTABLE_SQL_VERB_UPDATE "UPDATE "
#define _YTABLE_SQL_VERB_INSERT "INSERT INTO "
#define _YTABLE_SQL_VERB_DELETE "DELETE FROM "
#define _YTABLE_SQL_COMMA ", "
#define _YTABLE_SQL_KEYWORD_FROM " FROM "
#define _YTABLE_SQL_KEYWORD_SET " SET "
#define _YTABLE_SQL_KEYWORD_VALUES " VALUES "
#define _YTABLE_SQL_KEYWORD_WHERE " WHERE "
#define _YTABLE_SQL_KEYWORD_AND " AND "
#define _YTABLE_SQL_KEYWORD_OR " OR "
#define _YTABLE_SQL_OP_EQ " = "
#define _YTABLE_SQL_OP_NE " != "
#define _YTABLE_SQL_OP_GT " > "
#define _YTABLE_SQL_OP_GTE " >= "
#define _YTABLE_SQL_OP_LT " < "
#define _YTABLE_SQL_OP_LTE " <= "
#define _YTABLE_SQL_OP_IN " IN "
#define _YTABLE_SQL_OP_PLUS " + "
#define _YTABLE_SQL_OP_MINUS " - "
#define _YTABLE_SQL_QUOT_VALUE "'"
#define _YTABLE_SQL_QUOT_FIELD "`"
#define _YTABLE_SQL_BRACKET_LEFT "("
#define _YTABLE_SQL_BRACKET_RIGHT ")"
#define _YTABLE_SQL_INT_MAXLEN 20U

#define ytable_select(ytable, fields) _ytable_select((ytable), &(fields))
#define ytable_insert(ytable, values) _ytable_insert((ytable), &(values))
#define ytable_update(ytable, values) _ytable_update((ytable), &(values))
#define ytable_delete(ytable) _ytable_delete((ytable))
#define ytable_where(ytable, conditions) _ytable_where((ytable), &(conditions))
#define ytable_fetch_one(ytable, result) _ytable_fetch_one((ytable), &(result))
#define ytable_fetch_all(ytable, result) _ytable_fetch_all((ytable), &(result))
#define ytable_fetch_insert_id(ytable, insert_id) _ytable_fetch_insert_id((ytable), &(insert_id))

#define YTABLE_SELECT(ytable, ...) do { \
        yvar_t _raw_select_fields[] = { \
            __VA_ARGS__ \
        }; \
        yvar_t _select_fields = YVAR_ARRAY(_raw_select_fields); \
        ytable_select((ytable), _select_fields); \
    } while (0)

#define YTABLE_INSERT(ytable, ...) do { \
        yvar_map_kv_t _raw_inserts_fields = { \
            __VA_ARGS__ \
        }; \
        ytable_insert_map_kv((ytable), _raw_inserts_fields, sizeof(_raw_inserts_fields) / sizeof(_raw_inserts_fields[0])); \
    } while (0)

#define YTABLE_UPDATE(ytable, ...) do { \
        yvar_triple_array_t _raw_updates_fields = { \
            __VA_ARGS__ \
        }; \
        _ytable_update_triple_array((ytable), _raw_update_fields, sizeof(_raw_update_fields) / sizeof(_raw_updates_fields[0])); \
    } while (0)

#define YTABLE_DELETE(ytable) _ytable_delete((ytable))

ytable_t * ytable_instance(const char * table_name);
ytable_t * ytable_reset(ytable_t * ytable);
ytable_t * _ytable_select(ytable_t * ytable, const yvar_t * fields);
ytable_t * _ytable_insert(ytable_t * ytable, const yvar_t * values);
ytable_t * _ytable_insert_using_map_kv(ytable_t * ytable, yvar_map_kv_t values, ysize_t size);
ytable_t * _ytable_insert(ytable_t * ytable, const yvar_t * values);
ytable_t * _ytable_update(ytable_t * ytable, const yvar_t * values);
ytable_t * _ytable_update_using_triple_array(ytable_t * ytable, yvar_triple_array_t values, ysize_t size);
ytable_t * _ytable_delete(ytable_t * ytable);
ytable_t * _ytable_where(ytable_t * ytable, const yvar_t * conditions);
ytable_t * _ytable_where_using_triple_array(ytable_t * ytable, yvar_triple_array_t conditions, ysize_t size);

ybool_t _ytable_fetch_one(ytable_t * ytable, yvar_t ** result);
ybool_t _ytable_fetch_all(ytable_t * ytable, yvar_t ** result);

ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id);
ytable_error_t ytable_last_error(const ytable_t * ytable);

#ifdef __cplusplus
}
#endif

#endif
