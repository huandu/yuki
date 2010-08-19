#include <gtest/gtest.h>

#include "yuki.h"
#define YUKI_INI_FILE "./test/yuki.ini"

class YukiTableTest : public ::testing::Test {
protected:
    virtual void SetUp()
    {
        yuki_init(YUKI_INI_FILE);
    }

    virtual void TearDown()
    {
        yuki_shutdown();
    }
};

TEST_F(YukiTableTest, InsertOne) {
    yvar_t field1 = YVAR_EMPTY();
    yvar_t field2 = YVAR_EMPTY();
    yvar_t field3 = YVAR_EMPTY();
    yvar_t field4 = YVAR_EMPTY();
    yvar_t value1 = YVAR_EMPTY();
    yvar_t value2 = YVAR_EMPTY();
    yvar_t value3 = YVAR_EMPTY();
    yvar_t value4 = YVAR_EMPTY();
    yvar_t cond_key1 = YVAR_EMPTY();
    yvar_t cond_value1 = YVAR_EMPTY();
    yvar_cstr(field1, "uid");
    yvar_cstr(field2, "diamond");
    yvar_cstr(field3, "cash");
    yvar_cstr(field4, "created_at");
    yvar_cstr(value1, "1234567890");
    yvar_int64(value2, 21);
    yvar_int64(value3, 12);
    yvar_cstr(value4, "2010-08-13 01:23:45");
    yvar_cstr(cond_key1, "uid");
    yvar_cstr(cond_value1, "1234567890");

    yvar_map_kv_t raw_fields = {
        {field1, value1},
        {field2, value2},
        {field3, value3},
        {field4, value4},
    };

    yvar_t * insert_map;
    ASSERT_TRUE(yvar_map_smart_clone(insert_map, raw_fields));

    yvar_map_kv_t raw_cond = {
        {cond_key1, cond_value1},
    };
    yvar_t * cond;
    ASSERT_TRUE(yvar_map_smart_clone(cond, raw_cond));

    ytable_t * ytable = ytable_instance("mytest");
    ASSERT_TRUE(ytable);

    yvar_t * result;
    ASSERT_TRUE(ytable_insert(ytable, *insert_map));
    ASSERT_FALSE(ytable_where(ytable, *cond)); // cannot set where conf for INSERT
    ASSERT_TRUE(ytable_fetch_one(ytable, result));

    // the result should be a bool true:
    ASSERT_TRUE(yvar_is_bool(*result));
    yvar_t bool_true = YVAR_EMPTY();
    yvar_bool(bool_true, ytrue);
    ASSERT_TRUE(yvar_equal(*result, bool_true));
}

TEST_F(YukiTableTest, SelectOne) {
    yvar_t field1 = YVAR_EMPTY();
    yvar_t field2 = YVAR_EMPTY();
    yvar_t field3 = YVAR_EMPTY();
    yvar_t field4 = YVAR_EMPTY();
    yvar_t cond_key1 = YVAR_EMPTY();
    yvar_t cond_value1 = YVAR_EMPTY();
    yvar_cstr(field1, "uid");
    yvar_cstr(field2, "diamond");
    yvar_cstr(field3, "cash");
    yvar_cstr(field4, "created_at");
    yvar_cstr(cond_key1, "uid");
    yvar_cstr(cond_value1, "1234567890");

    yvar_t field_wildcard = YVAR_EMPTY();
    yvar_cstr(field_wildcard, "*");

    yvar_t raw_fields[] = {
        field_wildcard
    };

    yvar_t fields = YVAR_EMPTY();
    yvar_array(fields, raw_fields);

    yvar_map_kv_t raw_cond = {
        {cond_key1, cond_value1},
    };
    yvar_t * cond;
    ASSERT_TRUE(yvar_map_smart_clone(cond, raw_cond));

    ytable_t * ytable = ytable_instance("mytest");
    ASSERT_TRUE(ytable);

    yvar_t * result;
    ASSERT_TRUE(ytable_select(ytable, fields));
    ASSERT_TRUE(ytable_where(ytable, *cond));
    ASSERT_TRUE(ytable_fetch_one(ytable, result));

    // the result should be an array:
    // row #1 (a map):
    // - "uid" => "1234567890"
    // - "diamond => "21"
    // - "cash => "12"
    ASSERT_EQ(yvar_count(*result), 1u);

    yvar_t row1 = YVAR_EMPTY();
    ASSERT_TRUE(yvar_array_get(*result, 0, row1));
    ASSERT_TRUE(yvar_is_map(row1));

    yvar_t uid_var = YVAR_EMPTY();
    yvar_t diamond_var = YVAR_EMPTY();
    yvar_t cash_var = YVAR_EMPTY();
    yvar_t created_at_var = YVAR_EMPTY();

    ASSERT_TRUE(yvar_map_get(row1, field1, uid_var));
    ASSERT_TRUE(yvar_map_get(row1, field2, diamond_var));
    ASSERT_TRUE(yvar_map_get(row1, field3, cash_var));
    ASSERT_TRUE(yvar_map_get(row1, field4, created_at_var));

    yvar_t expected_uid_var = YVAR_EMPTY();
    yvar_t expected_diamond_var = YVAR_EMPTY();
    yvar_t expected_cash_var = YVAR_EMPTY();
    yvar_t expected_created_at = YVAR_EMPTY();
    yvar_cstr(expected_uid_var, "1234567890");
    yvar_int64(expected_diamond_var, 21);
    yvar_int64(expected_cash_var, 12);
    yvar_cstr(expected_created_at, "2010-08-13 01:23:45");

    ASSERT_TRUE(yvar_equal(uid_var, expected_uid_var));
    ASSERT_TRUE(yvar_equal(diamond_var, expected_diamond_var));
    ASSERT_TRUE(yvar_equal(cash_var, expected_cash_var));
    ASSERT_TRUE(yvar_equal(created_at_var, expected_created_at));
}

TEST_F(YukiTableTest, UpdateOne) {
    yvar_t field1 = YVAR_EMPTY();
    yvar_t field2 = YVAR_EMPTY();
    yvar_t field3 = YVAR_EMPTY();
    yvar_t value1 = YVAR_EMPTY();
    yvar_t value2 = YVAR_EMPTY();
    yvar_t value3 = YVAR_EMPTY();
    yvar_t cond_key1 = YVAR_EMPTY();
    yvar_t cond_value1 = YVAR_EMPTY();
    yvar_cstr(field1, "uid");
    yvar_cstr(field2, "diamond");
    yvar_cstr(field3, "cash");
    yvar_cstr(value1, "1234567890");
    yvar_int64(value2, 44444);
    yvar_int64(value3, 55555);
    yvar_cstr(cond_key1, "uid");
    yvar_cstr(cond_value1, "1234567890");

    yvar_map_kv_t raw_fields = {
        {field2, value2},
        {field3, value3},
    };

    yvar_t * update_map;
    ASSERT_TRUE(yvar_map_smart_clone(update_map, raw_fields));

    yvar_map_kv_t raw_cond = {
        {cond_key1, cond_value1},
    };
    yvar_t * cond;
    ASSERT_TRUE(yvar_map_smart_clone(cond, raw_cond));

    ytable_t * ytable = ytable_instance("mytest");
    ASSERT_TRUE(ytable);

    yvar_t * result;
    ASSERT_TRUE(ytable_update(ytable, *update_map));
    ASSERT_TRUE(ytable_where(ytable, *cond));
    ASSERT_TRUE(ytable_fetch_one(ytable, result));

    // the result should be a bool true:
    ASSERT_TRUE(yvar_is_bool(*result));
    yvar_t bool_true = YVAR_EMPTY();
    yvar_bool(bool_true, ytrue);
    ASSERT_TRUE(yvar_equal(*result, bool_true));
}


TEST_F(YukiTableTest, DeleteOne) {
    yvar_t cond_key1 = YVAR_EMPTY();
    yvar_t cond_value1 = YVAR_EMPTY();
    yvar_cstr(cond_key1, "uid");
    yvar_cstr(cond_value1, "1234567890");

    yvar_map_kv_t raw_cond = {
        {cond_key1, cond_value1},
    };
    yvar_t * cond;
    ASSERT_TRUE(yvar_map_smart_clone(cond, raw_cond));

    ytable_t * ytable = ytable_instance("mytest");
    ASSERT_TRUE(ytable);

    yvar_t * result;
    ASSERT_TRUE(ytable_delete(ytable));
    ASSERT_TRUE(ytable_where(ytable, *cond));
    ASSERT_TRUE(ytable_fetch_one(ytable, result));

    // the result should be a bool true:
    ASSERT_TRUE(yvar_is_bool(*result));
    yvar_t bool_true = YVAR_EMPTY();
    yvar_bool(bool_true, ytrue);
    ASSERT_TRUE(yvar_equal(*result, bool_true));
}

