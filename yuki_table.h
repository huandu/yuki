#ifndef _YUKI_TABLE_H_
#define _YUKI_TABLE_H_

#ifdef __cplusplus
extern "C" {
#endif

#define ytable(table_name, hash_key) _ytable(&(table_name), &(hash_key))

ytable_t * _ytable(const ycstr_t * table_name, const yvar_t * hash_key);

#ifdef __cplusplus
}
#endif

#endif
