#include <gtest/gtest.h>
#include "yuki.h"

#define YUKI_CFG_FILE "./test/yuki.config"

TEST(YukiVarTest, UseVar) {
    yuki_init(YUKI_CFG_FILE);

    #define _GENERATE_USE_VAR_CASE(t, v) do { \
        yvar_t my_var = YVAR_EMPTY(); \
        y##t##_t exp_value = (v); \
        yvar_##t(my_var, exp_value); \
        y##t##_t value; \
    \
        ASSERT_TRUE(yvar_get_##t(my_var, value)) << "cannot get "#t" type value"; \
        ASSERT_EQ(exp_value, value) << "wrong "#t" type value"; \
    } while (0)

    _GENERATE_USE_VAR_CASE(bool, ytrue);
    _GENERATE_USE_VAR_CASE(int8, 12);
    _GENERATE_USE_VAR_CASE(uint8, 200);
    _GENERATE_USE_VAR_CASE(int16, 23456);
    _GENERATE_USE_VAR_CASE(uint16, 64727);
    _GENERATE_USE_VAR_CASE(int32, 78901234);
    _GENERATE_USE_VAR_CASE(uint32, 0x93123452UL);
    _GENERATE_USE_VAR_CASE(int64, 0x7342930284728340LL);
    _GENERATE_USE_VAR_CASE(uint64, 0xE03AE8439DCC2194ULL);

    {
        yvar_t my_var = YVAR_EMPTY();
        const char exp_value[] = "Hello world";
        yvar_cstr(my_var, exp_value);
        char buffer[20];

        ASSERT_TRUE(yvar_get_cstr(my_var, buffer, sizeof(buffer)));
        ASSERT_EQ(0, strcmp(exp_value, buffer));
    }

    #undef _GENERATE_USE_VAR_CASE

    yuki_clean_up();
    yuki_shutdown();
}

TEST(YukiVarTest, ForeachVarArray) {
    yuki_init(YUKI_CFG_FILE);

    #define _GENERATE_FOREACH_VAR_CASE(t, v) \
        yvar_t var_##t = YVAR_EMPTY(); \
        y##t##_t exp_##t = (v); \
        yvar_##t(var_##t, exp_##t); \

    _GENERATE_FOREACH_VAR_CASE(bool, ytrue);
    _GENERATE_FOREACH_VAR_CASE(int8, 12);
    _GENERATE_FOREACH_VAR_CASE(uint8, 200);
    _GENERATE_FOREACH_VAR_CASE(int16, 23456);
    _GENERATE_FOREACH_VAR_CASE(uint16, 64727);
    _GENERATE_FOREACH_VAR_CASE(int32, 78901234);
    _GENERATE_FOREACH_VAR_CASE(uint32, 0x93123452UL);
    _GENERATE_FOREACH_VAR_CASE(int64, 0x7342930284728340LL);
    _GENERATE_FOREACH_VAR_CASE(uint64, 0xE03AE8439DCC2194ULL);
    yvar_t var_cstr = YVAR_EMPTY();
    char exp_cstr[] = "Hello world";
    yvar_cstr(var_cstr, exp_cstr);

    yvar_t raw_arr[] = {
        var_bool, var_int8, var_uint8, var_int16, var_uint16,
        var_int32, var_uint32, var_int64, var_uint64, var_cstr,
    };
    yvar_t arr = YVAR_EMPTY();
    yvar_array(arr, raw_arr);

    ASSERT_EQ(10u, yvar_count(arr));

    yvar_t output = YVAR_EMPTY();

    ASSERT_TRUE(yvar_array_get(arr, 0, output));
    ASSERT_TRUE(yvar_equal(output, var_bool));
    ASSERT_TRUE(yvar_array_get(arr, 1, output));
    ASSERT_TRUE(yvar_equal(output, var_int8));
    ASSERT_TRUE(yvar_array_get(arr, 2, output));
    ASSERT_TRUE(yvar_equal(output, var_uint8));
    ASSERT_TRUE(yvar_array_get(arr, 3, output));
    ASSERT_TRUE(yvar_equal(output, var_int16));
    ASSERT_TRUE(yvar_array_get(arr, 4, output));
    ASSERT_TRUE(yvar_equal(output, var_uint16));
    ASSERT_TRUE(yvar_array_get(arr, 5, output));
    ASSERT_TRUE(yvar_equal(output, var_int32));
    ASSERT_TRUE(yvar_array_get(arr, 6, output));
    ASSERT_TRUE(yvar_equal(output, var_uint32));
    ASSERT_TRUE(yvar_array_get(arr, 7, output));
    ASSERT_TRUE(yvar_equal(output, var_int64));
    ASSERT_TRUE(yvar_array_get(arr, 8, output));
    ASSERT_TRUE(yvar_equal(output, var_uint64));
    ASSERT_TRUE(yvar_array_get(arr, 9, output));
    ASSERT_TRUE(yvar_equal(output, var_cstr));

    yvar_t undefined = YVAR_EMPTY();
    yvar_undefined(undefined);
    ASSERT_TRUE(yvar_array_get(arr, 10, output));
    ASSERT_TRUE(yvar_equal(output, undefined));

    #undef _GENERATE_FOREACH_VAR_CASE

    yuki_clean_up();
    yuki_shutdown();
}

TEST(YukiVarTest, VarClone) {
    yvar_t * before_init_var = NULL;
    yvar_t before_init_int8_var = YVAR_EMPTY();
    yvar_int8(before_init_int8_var, 127);
    ASSERT_FALSE(yvar_clone(before_init_var, before_init_int8_var));

    yuki_init(YUKI_CFG_FILE);

    #define _GENERATE_VAR_CLONE_CASE(t, v) do { \
        yvar_t yvar = YVAR_EMPTY(); \
        yvar_t * new_var = NULL; \
        yvar_##t(yvar, (v)); \
        ASSERT_TRUE(yvar_clone(new_var, yvar)); \
        ASSERT_TRUE(yvar_equal(*new_var, yvar)); \
    } while (0)

    _GENERATE_VAR_CLONE_CASE(bool, ytrue);
    _GENERATE_VAR_CLONE_CASE(int8, 12);
    _GENERATE_VAR_CLONE_CASE(uint8, 200);
    _GENERATE_VAR_CLONE_CASE(int16, 23456);
    _GENERATE_VAR_CLONE_CASE(uint16, 64727);
    _GENERATE_VAR_CLONE_CASE(int32, 78901234);
    _GENERATE_VAR_CLONE_CASE(uint32, 0x93123452UL);
    _GENERATE_VAR_CLONE_CASE(int64, 0x7342930284728340LL);
    _GENERATE_VAR_CLONE_CASE(uint64, 0xE03AE8439DCC2194ULL);

    {
        yvar_t * new_var = NULL;
        yvar_t yvar = YVAR_EMPTY();
        char exp_cstr[] = "Hello world";
        yvar_cstr(yvar, exp_cstr);
        ASSERT_TRUE(yvar_clone(new_var, yvar));
        ASSERT_FALSE(new_var == NULL);
        ASSERT_TRUE(yvar_equal(*new_var, yvar));
    }

    {
        yvar_t * new_var = NULL;
        yvar_t yvar1 = YVAR_EMPTY();
        yvar_t yvar2 = YVAR_EMPTY();
        yvar_t yvar3 = YVAR_EMPTY();
        yvar_t yvar4 = YVAR_EMPTY();
        yvar_t yvar5 = YVAR_EMPTY();
        yvar_t yvar6 = YVAR_EMPTY();
        yvar_t yvar7 = YVAR_EMPTY();
        yvar_t yvar8 = YVAR_EMPTY();
        char exp_cstr1[] = "Hello world";
        char exp_cstr2[] = "Hello world 2nd";
        char exp_cstr3[] = "Hello world 3rd";
        char exp_cstr4[] = "Hello world 4th";
        char exp_cstr5[] = "Hello world 5th";
        char exp_cstr6[] = "Hello world 6th";
        char exp_cstr7[] = "Hello world 7th";
        char exp_cstr8[] = "Hello world 8th";
        yvar_cstr(yvar1, exp_cstr1);
        yvar_cstr(yvar2, exp_cstr2);
        yvar_cstr(yvar3, exp_cstr3);
        yvar_cstr(yvar4, exp_cstr4);
        yvar_cstr(yvar5, exp_cstr5);
        yvar_cstr(yvar6, exp_cstr6);
        yvar_cstr(yvar7, exp_cstr7);
        yvar_cstr(yvar8, exp_cstr8);

        yvar_t raw_arr1[] = {
            yvar1, yvar2, yvar3, yvar4
        };
        yvar_t raw_arr2[] = {
            yvar5, yvar6, yvar7, yvar8
        };

        yvar_t arr1 = YVAR_EMPTY();
        yvar_array(arr1, raw_arr1);
        ASSERT_TRUE(yvar_clone(new_var, arr1));
        ASSERT_TRUE(yvar_equal(*new_var, arr1));
        ASSERT_EQ(yvar_count(*new_var), 4u);

        ysize_t i = 0;

        FOREACH_YVAR_ARRAY(arr1, value1) {
            ASSERT_TRUE(yvar_equal(*value1, raw_arr1[i]));
            i++;
        }

        yvar_t arr2 = YVAR_EMPTY();
        yvar_array(arr2, raw_arr2);
        yvar_t map = YVAR_EMPTY();
        yvar_map(map, arr2, arr1);
        ASSERT_TRUE(yvar_clone(new_var, map));
        ASSERT_TRUE(yvar_equal(*new_var, map));
        ASSERT_EQ(yvar_count(*new_var), 4u);

        i = 0;

        FOREACH_YVAR_MAP(map, key2, value2) {
            ASSERT_TRUE(yvar_equal(*key2, raw_arr2[i]));
            ASSERT_TRUE(yvar_equal(*value2, raw_arr1[i]));
            i++;
        }

        yvar_t list = YVAR_EMPTY();
        yvar_list(list);
        yvar_t raw_arr3[] = {
            yvar1, yvar2, yvar3, yvar4, arr2, yvar6, yvar7, map
        };
        ysize_t cnt = sizeof(raw_arr3) / sizeof(raw_arr3[0]);

        for (i = 0; i < cnt; i++) {
            ASSERT_TRUE(yvar_list_push_back(list, raw_arr3[i]));
        }

        ASSERT_EQ(yvar_count(list), cnt);
        ASSERT_TRUE(yvar_clone(new_var, list));
        ASSERT_TRUE(yvar_equal(*new_var, list));
        ASSERT_EQ(yvar_count(*new_var), cnt);

        i = 0;

        FOREACH_YVAR_LIST(list, value3) {
            ASSERT_TRUE(yvar_equal(*value3, raw_arr3[i]));
            i++;
        }
    }

    #undef _GENERATE_VAR_CLONE_CASE

    yuki_clean_up();
    yuki_shutdown();
}

TEST(YukiVarTest, VarPinAndUnpin) {
    yvar_t * before_init_var = NULL;
    yvar_t before_init_int8_var = YVAR_EMPTY();
    yvar_int8(before_init_int8_var, 127);
    ASSERT_FALSE(yvar_pin(before_init_var, before_init_int8_var));

    yuki_init(YUKI_CFG_FILE);

    #define _GENERATE_VAR_PIN_CASE(t, v) do { \
        yvar_t yvar = YVAR_EMPTY(); \
        yvar_t * new_var = NULL; \
        yvar_##t(yvar, (v)); \
        ASSERT_TRUE(yvar_pin(new_var, yvar)); \
        ASSERT_TRUE(yvar_equal(*new_var, yvar)); \
        \
        yuki_clean_up(); \
        ASSERT_TRUE(yvar_unpin(new_var)); \
        ASSERT_FALSE(yvar_unpin(new_var)); \
        ASSERT_TRUE(yvar_pin(new_var, yvar)); \
        ASSERT_TRUE(yvar_equal(*new_var, yvar)); \
        \
        yuki_shutdown(); \
        ASSERT_FALSE(yvar_unpin(new_var)); \
        yuki_init(YUKI_CFG_FILE); \
    } while (0)

    _GENERATE_VAR_PIN_CASE(bool, ytrue);
    _GENERATE_VAR_PIN_CASE(int8, 12);
    _GENERATE_VAR_PIN_CASE(uint8, 200);
    _GENERATE_VAR_PIN_CASE(int16, 23456);
    _GENERATE_VAR_PIN_CASE(uint16, 64727);
    _GENERATE_VAR_PIN_CASE(int32, 78901234);
    _GENERATE_VAR_PIN_CASE(uint32, 0x93123452UL);
    _GENERATE_VAR_PIN_CASE(int64, 0x7342930284728340LL);
    _GENERATE_VAR_PIN_CASE(uint64, 0xE03AE8439DCC2194ULL);

    {
        yvar_t * new_var = NULL;
        yvar_t yvar = YVAR_EMPTY();
        char exp_cstr[] = "Hello world";
        yvar_cstr(yvar, exp_cstr);
        ASSERT_TRUE(yvar_pin(new_var, yvar));
        ASSERT_FALSE(new_var == NULL);
        ASSERT_TRUE(yvar_equal(*new_var, yvar));

        yuki_clean_up();
        ASSERT_TRUE(yvar_unpin(new_var));
        ASSERT_FALSE(yvar_unpin(new_var));

        ASSERT_TRUE(yvar_pin(new_var, yvar));
        ASSERT_FALSE(new_var == NULL);
        ASSERT_TRUE(yvar_equal(*new_var, yvar));
        yuki_shutdown();
        ASSERT_FALSE(yvar_unpin(new_var));
        yuki_init(YUKI_CFG_FILE);
    }

    {
        yvar_t * new_var = NULL;
        yvar_t yvar1 = YVAR_EMPTY();
        yvar_t yvar2 = YVAR_EMPTY();
        yvar_t yvar3 = YVAR_EMPTY();
        yvar_t yvar4 = YVAR_EMPTY();
        yvar_t yvar5 = YVAR_EMPTY();
        yvar_t yvar6 = YVAR_EMPTY();
        yvar_t yvar7 = YVAR_EMPTY();
        yvar_t yvar8 = YVAR_EMPTY();
        char exp_cstr1[] = "Hello world";
        char exp_cstr2[] = "Hello world 2nd";
        char exp_cstr3[] = "Hello world 3rd";
        char exp_cstr4[] = "Hello world 4th";
        char exp_cstr5[] = "Hello world 5th";
        char exp_cstr6[] = "Hello world 6th";
        char exp_cstr7[] = "Hello world 7th";
        char exp_cstr8[] = "Hello world 8th";
        yvar_cstr(yvar1, exp_cstr1);
        yvar_cstr(yvar2, exp_cstr2);
        yvar_cstr(yvar3, exp_cstr3);
        yvar_cstr(yvar4, exp_cstr4);
        yvar_cstr(yvar5, exp_cstr5);
        yvar_cstr(yvar6, exp_cstr6);
        yvar_cstr(yvar7, exp_cstr7);
        yvar_cstr(yvar8, exp_cstr8);

        yvar_t raw_arr1[] = {
            yvar1, yvar2, yvar3, yvar4
        };
        yvar_t raw_arr2[] = {
            yvar5, yvar6, yvar7, yvar8
        };

        yvar_t arr1 = YVAR_EMPTY();
        yvar_array(arr1, raw_arr1);
        ASSERT_TRUE(yvar_pin(new_var, arr1));
        ASSERT_TRUE(yvar_equal(*new_var, arr1));
        ASSERT_EQ(yvar_count(*new_var), 4u);

        ysize_t i = 0;

        FOREACH_YVAR_ARRAY(arr1, value1) {
            ASSERT_TRUE(yvar_equal(*value1, raw_arr1[i]));
            i++;
        }

        yuki_clean_up();
        ASSERT_TRUE(yvar_unpin(new_var));
        ASSERT_FALSE(yvar_unpin(new_var));
        
        ASSERT_TRUE(yvar_pin(new_var, arr1));
        ASSERT_TRUE(yvar_equal(*new_var, arr1));
        yuki_shutdown();
        ASSERT_FALSE(yvar_unpin(new_var));
        yuki_init(YUKI_CFG_FILE);

        yvar_t arr2 = YVAR_EMPTY();
        yvar_array(arr2, raw_arr2);
        yvar_t map = YVAR_EMPTY();
        yvar_map(map, arr2, arr1);
        ASSERT_TRUE(yvar_pin(new_var, map));
        ASSERT_TRUE(yvar_equal(*new_var, map));
        ASSERT_EQ(yvar_count(*new_var), 4u);

        i = 0;

        FOREACH_YVAR_MAP(map, key2, value2) {
            ASSERT_TRUE(yvar_equal(*key2, raw_arr2[i]));
            ASSERT_TRUE(yvar_equal(*value2, raw_arr1[i]));
            i++;
        }

        yuki_clean_up();
        ASSERT_TRUE(yvar_unpin(new_var));
        ASSERT_FALSE(yvar_unpin(new_var));
        
        ASSERT_TRUE(yvar_pin(new_var, map));
        ASSERT_TRUE(yvar_equal(*new_var, map));
        yuki_shutdown();
        ASSERT_FALSE(yvar_unpin(new_var));
        yuki_init(YUKI_CFG_FILE);

        yvar_t * new_list = NULL;
        yvar_t list = YVAR_EMPTY();
        yvar_list(list);
        yvar_t raw_arr3[] = {
            yvar1, yvar2, yvar3, yvar4, arr2, yvar6, yvar7, map
        };
        ysize_t cnt = sizeof(raw_arr3) / sizeof(raw_arr3[0]);

        for (i = 0; i < cnt; i++) {
            ASSERT_TRUE(yvar_list_push_back(list, raw_arr3[i]));
        }

        ASSERT_EQ(yvar_count(list), cnt);
        ASSERT_TRUE(yvar_pin(new_var, list));
        ASSERT_TRUE(yvar_pin(new_list, list));
        ASSERT_TRUE(yvar_equal(*new_var, list));
        ASSERT_TRUE(yvar_equal(*new_list, list));
        ASSERT_EQ(yvar_count(*new_var), cnt);
        ASSERT_EQ(yvar_count(*new_list), cnt);

        i = 0;

        FOREACH_YVAR_LIST(list, value3) {
            ASSERT_TRUE(yvar_equal(*value3, raw_arr3[i]));
            i++;
        }

        yuki_clean_up();
        ASSERT_TRUE(yvar_unpin(new_var));
        ASSERT_FALSE(yvar_unpin(new_var));

        ASSERT_TRUE(yvar_pin(new_var, *new_list));
        ASSERT_TRUE(yvar_equal(*new_var, *new_list));
        ASSERT_TRUE(yvar_unpin(new_list));
        yuki_shutdown();
        ASSERT_FALSE(yvar_unpin(new_var));
        yuki_init(YUKI_CFG_FILE);
    }

    #undef _GENERATE_VAR_CLONE_CASE

    yuki_clean_up();
    yuki_shutdown();
}

TEST(YukiVarTest, VarMapCloneAndPin) {
    yuki_init(YUKI_CFG_FILE);

    yvar_t * new_var = NULL;
    yvar_t yvar1 = YVAR_EMPTY();
    yvar_t yvar2 = YVAR_EMPTY();
    yvar_t yvar3 = YVAR_EMPTY();
    yvar_t yvar4 = YVAR_EMPTY();
    yvar_t yvar5 = YVAR_EMPTY();
    yvar_t yvar6 = YVAR_EMPTY();
    yvar_t yvar7 = YVAR_EMPTY();
    yvar_t yvar8 = YVAR_EMPTY();
    char exp_cstr1[] = "Hello world";
    char exp_cstr2[] = "Hello world 2nd";
    char exp_cstr3[] = "Hello world 3rd";
    char exp_cstr4[] = "Hello world 4th";
    char exp_cstr5[] = "Hello world 5th";
    char exp_cstr6[] = "Hello world 6th";
    char exp_cstr7[] = "Hello world 7th";
    char exp_cstr8[] = "Hello world 8th";
    yvar_cstr(yvar1, exp_cstr1);
    yvar_cstr(yvar2, exp_cstr2);
    yvar_cstr(yvar3, exp_cstr3);
    yvar_cstr(yvar4, exp_cstr4);
    yvar_cstr(yvar5, exp_cstr5);
    yvar_cstr(yvar6, exp_cstr6);
    yvar_cstr(yvar7, exp_cstr7);
    yvar_cstr(yvar8, exp_cstr8);

    yvar_t raw_arr1[] = {
        yvar1, yvar2, yvar3, yvar4
    };
    yvar_t raw_arr2[] = {
        yvar5, yvar6, yvar7, yvar8
    };
    yvar_map_kv_t raw_key_value = {
        {yvar1, yvar5},
        {yvar2, yvar6},
        {yvar3, yvar7},
        {yvar4, yvar8},
    };
    yvar_t keys = YVAR_EMPTY();
    yvar_array(keys, raw_arr1);
    yvar_t values = YVAR_EMPTY();
    yvar_array(values, raw_arr2);
    yvar_t map = YVAR_EMPTY();
    yvar_map(map, keys, values);
    
    ASSERT_TRUE(yvar_map_smart_clone(new_var, raw_key_value));
    ASSERT_TRUE(yvar_equal(*new_var, map));
    ASSERT_TRUE(yvar_map_smart_pin(new_var, raw_key_value));
    ASSERT_TRUE(yvar_equal(*new_var, map));
    ASSERT_TRUE(yvar_unpin(new_var));

    yuki_shutdown();
}

TEST(YukiVarTest, VarArrayOfArrayCloneAndPin) {
    yuki_init(YUKI_CFG_FILE);

    yvar_t * new_var = NULL;
    yvar_t yvar1 = YVAR_EMPTY();
    yvar_t yvar2 = YVAR_EMPTY();
    yvar_t yvar3 = YVAR_EMPTY();
    yvar_t yvar4 = YVAR_EMPTY();
    yvar_t yvar5 = YVAR_EMPTY();
    yvar_t yvar6 = YVAR_EMPTY();
    yvar_t yvar7 = YVAR_EMPTY();
    yvar_t yvar8 = YVAR_EMPTY();
    char exp_cstr1[] = "Hello world";
    char exp_cstr2[] = "Hello world 2nd";
    char exp_cstr3[] = "Hello world 3rd";
    char exp_cstr4[] = "Hello world 4th";
    char exp_cstr5[] = "Hello world 5th";
    char exp_cstr6[] = "Hello world 6th";
    char exp_cstr7[] = "Hello world 7th";
    char exp_cstr8[] = "Hello world 8th";
    yvar_cstr(yvar1, exp_cstr1);
    yvar_cstr(yvar2, exp_cstr2);
    yvar_cstr(yvar3, exp_cstr3);
    yvar_cstr(yvar4, exp_cstr4);
    yvar_cstr(yvar5, exp_cstr5);
    yvar_cstr(yvar6, exp_cstr6);
    yvar_cstr(yvar7, exp_cstr7);
    yvar_cstr(yvar8, exp_cstr8);

    yvar_t raw_arr1[] = {
        yvar1, yvar2, yvar3
    };
    yvar_t raw_arr2[] = {
        yvar5, yvar6, yvar7
    };
    yvar_triple_array_t raw_array_of_array = {
        {yvar1, yvar2, yvar3},
        {yvar5, yvar6, yvar7},
    };
    yvar_t field1 = YVAR_EMPTY();
    yvar_array(field1, raw_arr1);
    yvar_t field2 = YVAR_EMPTY();
    yvar_array(field2, raw_arr2);
    yvar_t expected_raw_array_of_array[] = {
        field1, field2
    };
    yvar_t expected_array_of_array = YVAR_EMPTY();
    yvar_array(expected_array_of_array, expected_raw_array_of_array);

    ASSERT_TRUE(yvar_triple_array_smart_clone(new_var, raw_array_of_array));
    ASSERT_TRUE(yvar_equal(*new_var, expected_array_of_array));
    ASSERT_TRUE(yvar_triple_array_smart_pin(new_var, raw_array_of_array));
    ASSERT_TRUE(yvar_equal(*new_var, expected_array_of_array));
    ASSERT_TRUE(yvar_unpin(new_var));

    yuki_shutdown();
}

