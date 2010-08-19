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
#define ytable_fetch_insert_id(ytable, insert_id) _ytable_fetch_insert_id((ytable), &(insert_id))

ytable_t * ytable_instance(const char * table_name);
ytable_t * ytable_reset(ytable_t * ytable);
ybool_t _ytable_select(ytable_t * ytable, const yvar_t * fields);
ybool_t _ytable_insert(ytable_t * ytable, const yvar_t * values);
ybool_t _ytable_update(ytable_t * ytable, const yvar_t * values);
ybool_t _ytable_delete(ytable_t * ytable);
ybool_t _ytable_where(ytable_t * ytable, const yvar_t * conditions);
ybool_t _ytable_fetch_one(ytable_t * ytable, yvar_t ** result);
ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id);

#ifdef __cplusplus
}
#endif

#endif
