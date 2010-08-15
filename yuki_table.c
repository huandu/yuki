#include "yuki.h"

ytable_t * ytable_instance(const char * table_name)
{
    return NULL;
}

ybool_t _ytable_select(ytable_t * ytable, const yvar_t * fields)
{
    return yfalse;
}

ybool_t _ytable_insert(ytable_t * ytable, const ymap_t * clause)
{
    return yfalse;
}

ybool_t _ytable_update(ytable_t * ytable, const ymap_t * clause)
{
    return yfalse;
}

ybool_t _ytable_delete(ytable_t * ytable)
{
    return yfalse;
}

ybool_t _ytable_where(ytable_t * ytable, const ymap_t * conditions)
{
    return yfalse;
}

ybool_t _ytable_fetch_one(ytable_t * ytable, ymap_t * result)
{
    return yfalse;
}

ybool_t _ytable_fetch_insert_id(ytable_t * ytable, yvar_t * insert_id)
{
    return yfalse;
}

