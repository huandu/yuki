#ifndef _YUKI_TABLE_H_
#define _YUKI_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ytable_select(ytable, fields) _ytable_select((ytable), &(fields))
#define ytable_insert(ytable, ymap_t) _ytable_insert((ytable), &(ymap_t))
#define ytable_update(ytable, ymap_t) _ytable_update((ytable), &(ymap_t))
#define ytable_delete(ytable) _ytable_delete((ytable))
#define ytable_where(ytable, conditions) _ytable_where((ytable), &(conditions))
#define ytable_fetch_one(ytable, result) _ytable_fetch_one((ytable), &(result))
#define ytable_fetch_insert_id(ytable, insert_id) _ytable_fetch_insert_id((ytable), &(insert_id))

ytable_t * ytable_instance(const char * table_name);
ybool_t _ytable_select(ytable_t * ytable, const yvar_t * fields);
ybool_t _ytable_insert(ytable_t * ytable, const ymap_t * clause);
ybool_t _ytable_update(ytable_t * ytable, const ymap_t * clause);
ybool_t _ytable_delete(ytable_t * ytable);
ybool_t _ytable_where(ytable_t * ytable, const ymap_t * conditions);
ybool_t _ytable_fetch_one(ytable_t * ytable, ymap_t * result);
ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id);

#ifdef __cplusplus
}
#endif

#endif
