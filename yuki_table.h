#ifndef _YUKI_TABLE_H_
#define _YUKI_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ytable_select(ytable, fields) _ytable_select((ytable), &(fields))
#define ytable_where(ytable, condition, params) _ytable_where((ytable), &(condition), &(params))

ytable_t * ytable_instance(const char * table_name);
ybool_t _ytable_select(ytable_t * ytable, const yvar_t * fields);
ybool_t _ytable_insert(ytable_t * ytable, const yvar_t * ymap_t);
ybool_t _ytable_update(ytable_t * ytable, const yvar_t * ymap_t);
ybool_t _ytable_delete(ytable_t * ytable);
ybool_t _ytable_where(ytable_t * ytable, const yvar_t * condition, const yvar_t * params);
ybool_t _ytable_fetch_result(ytable_t * ytable, yvar_t * ymap_t);

#ifdef __cplusplus
}
#endif

#endif
