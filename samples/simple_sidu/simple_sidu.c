#include <stdlib.h>
#include <stdio.h>

#include "yuki.h"

#define SAMPLE_TABLE_NAME "mysample"

int main(int argc, char * argv[])
{
    if (argc != 2) {
        YUKI_LOG_TRACE("ERROR: must have 1 param which is the yuki.ini path");
        return -1;
    }

    YUKI_LOG_TRACE("please make sure to import sample.sql to your database 'test' before using this sample");

    yuki_init(argv[1]);
    atexit(&yuki_shutdown);

    ///////////////////////////////////////////////////////////////////
    YUKI_LOG_TRACE("start to do insert...");

    yvar_map_kv_t insert_raw_fields = {
        {YVAR_CSTR("uid"),                   YVAR_CSTR("huandu")},
        {YVAR_CSTR("int_value"),             YVAR_INT32(12345)},
        {YVAR_CSTR("bigint_unsigned_value"), YVAR_UINT64(0xFFEECCDDBBAA0099UL)},
        {YVAR_CSTR("char_value"),            YVAR_CSTR("hey~")},
        {YVAR_CSTR("varchar_value"),         YVAR_CSTR("hello")},
        {YVAR_CSTR("text_value"),            YVAR_CSTR("world")},
    };

    yvar_t * insert_fields;
    yvar_map_smart_clone(insert_fields, insert_raw_fields);

    ytable_t * ytable = NULL;
    yvar_t * result = NULL;

    ytable = ytable_instance(SAMPLE_TABLE_NAME);
    ytable_insert(ytable, *insert_fields);
    ybool_t ret = ytable_fetch_one(ytable, result);
    yvar_t yvar_true = YVAR_BOOL(ytrue);

    if (!ret || !yvar_equal(*result, yvar_true)) {
        YUKI_LOG_TRACE("fail to do insert");
        return -2;
    }

    ///////////////////////////////////////////////////////////////////
    YUKI_LOG_TRACE("start to do select...");

    yvar_t select_raw_fields[] = {
        YVAR_CSTR("*")
    };
    yvar_t select_fields = YVAR_ARRAY(select_raw_fields);

    yvar_map_kv_t select_raw_cond = {
        {YVAR_CSTR("uid"), YVAR_CSTR("huandu")},
    };
    yvar_t * select_cond = NULL;
    yvar_map_smart_clone(select_cond, select_raw_cond);

    ytable = ytable_reset(ytable);
    ytable_select(ytable, select_fields);
    ytable_where(ytable, *select_cond);
    ret = ytable_fetch_one(ytable, result);

    if (!ret || !yvar_is_array(*result) || yvar_count(*result) != 1) {
        YUKI_LOG_TRACE("fail to do select");
        return -3;
    }

    yvar_t row = YVAR_EMPTY();
    yvar_array_get(*result, 0, row);
    char value_buffer[256];

    FOREACH_YVAR_MAP(row, key, value) {
        yvar_get_cstr(*value, value_buffer, sizeof(value_buffer));

        YUKI_LOG_TRACE("got one field. [key: %s] [value: %s]",
            yvar_cstr_buffer(*key), value_buffer);
    }

    ///////////////////////////////////////////////////////////////////
    YUKI_LOG_TRACE("start to do update...");

    yvar_map_kv_t update_raw_fields = {
        {YVAR_CSTR("int_value"), YVAR_INT32(54321)},
        {YVAR_CSTR("text_value"), YVAR_INT32(4567788)}, // yes, yuki will convert number to string automatically!
    };
    yvar_t * update_fields;
    yvar_map_smart_clone(update_fields, update_raw_fields);

    ytable = ytable_reset(ytable);
    ytable_update(ytable, *update_fields);
    ytable_where(ytable, *select_cond);
    ret = ytable_fetch_one(ytable, result);

    if (!ret || !yvar_equal(*result, yvar_true)) {
        YUKI_LOG_TRACE("fail to do update");
        return -4;
    }

    ///////////////////////////////////////////////////////////////////
    YUKI_LOG_TRACE("start to do delete...");

    ytable = ytable_reset(ytable);
    ytable_delete(ytable);
    ytable_where(ytable, *select_cond);
    ret = ytable_fetch_one(ytable, result);

    if (!ret || !yvar_equal(*result, yvar_true)) {
        YUKI_LOG_TRACE("fail to do delete");
        return -5;
    }

    YUKI_LOG_TRACE("Hooray!!!! Sample runs successfully!!!");

    return 0;
}
