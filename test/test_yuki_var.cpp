#include <gtest/gtest.h>
#include "yuki.h"

TEST(YukiVarTest, UseVar) {
    yuki_log_init();

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
}

TEST(YukiVarTest, ForeachVarArray) {
    yuki_log_init();

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
    ASSERT_TRUE(yvar_is_equal(output, var_bool));
    ASSERT_TRUE(yvar_array_get(arr, 1, output));
    ASSERT_TRUE(yvar_is_equal(output, var_int8));
    ASSERT_TRUE(yvar_array_get(arr, 2, output));
    ASSERT_TRUE(yvar_is_equal(output, var_uint8));
    ASSERT_TRUE(yvar_array_get(arr, 3, output));
    ASSERT_TRUE(yvar_is_equal(output, var_int16));
    ASSERT_TRUE(yvar_array_get(arr, 4, output));
    ASSERT_TRUE(yvar_is_equal(output, var_uint16));
    ASSERT_TRUE(yvar_array_get(arr, 5, output));
    ASSERT_TRUE(yvar_is_equal(output, var_int32));
    ASSERT_TRUE(yvar_array_get(arr, 6, output));
    ASSERT_TRUE(yvar_is_equal(output, var_uint32));
    ASSERT_TRUE(yvar_array_get(arr, 7, output));
    ASSERT_TRUE(yvar_is_equal(output, var_int64));
    ASSERT_TRUE(yvar_array_get(arr, 8, output));
    ASSERT_TRUE(yvar_is_equal(output, var_uint64));
    ASSERT_TRUE(yvar_array_get(arr, 9, output));
    ASSERT_TRUE(yvar_is_equal(output, var_cstr));

    yvar_t undefined = YVAR_EMPTY();
    yvar_undefined(undefined);
    ASSERT_TRUE(yvar_array_get(arr, 10, output));
    ASSERT_TRUE(yvar_is_equal(output, undefined));

    #undef _GENERATE_FOREACH_VAR_CASE
}

#if 0
    printf("var data is: %d\n", yvar.data.yint8_data);
    printf("pvar data is: %d\n", pyvar[0].data.yint16_data);
    
    ycstr_t cstr = YCSTR("Hello world!");
    printf("cstr data is: %s, size is %lu\n", cstr.str, cstr.size);
    
    yvar_t cstr_var = YVAR_CSTR("Hey you!");
    printf("cstr_var data is: %s, size is %lu\n", cstr_var.data.ycstr_data.str, cstr_var.data.ycstr_data.size);
    
    yvar_t yarr[] = {YVAR_INT8(23), YVAR_INT16(4444), YVAR_CSTR("Who?")};
    yvar_t yarr_key = YVAR_ARRAY(yarr);
    
    yvar_t yarr2[] = {YVAR_INT8(14), YVAR_INT16(2345), YVAR_CSTR("Me!")};
    yvar_t yarr_value = YVAR_ARRAY(yarr2);

    yvar_t yassign;
    yvar_assign(yassign, cstr_var);
    printf("assigned data is: %s, size is %lu\n", yassign.data.ycstr_data.str, yassign.data.ycstr_data.size);

    yvar_t * pnull = NULL;
    yvar_assign(*pnull, cstr_var); // nothing happens

    char *is_or_not[] = {"is not", "is"};
    printf("var data %s arr\n", is_or_not[yvar_is_array(yvar)]);
    printf("cstr data %s arr\n", is_or_not[yvar_is_array(cstr_var)]);
    printf("arr data %s arr\n", is_or_not[yvar_is_array(yarr_key)]);
    printf("arr data %s arr\n", is_or_not[yvar_is_array(yarr_value)]);

    // if not using C99, need to use this extra {}
    {
        YVAR_FOREACH(yarr_key, value) {
            printf("item: ");

            switch (value->type) {
                case YVAR_TYPE_INT8:
                    printf("data is %d\n", value->data.yint8_data);
                    break;

                case YVAR_TYPE_INT16:
                    printf("data is %d\n", value->data.yint16_data);
                    break;
                case YVAR_TYPE_CSTR:
                    printf("data is %s\n", value->data.ycstr_data.str);
                    break;
                default:
                    printf("unknown data type\n");
                    break;
            }
        }
    }

    {
        // do foreach on a non-arr var will simply do nothing
        YVAR_FOREACH(yvar, value) {
            // won't be here
            printf("item: ");
        }
    }
    
    ymap_t map = YMAP_CREATE(yarr_key, yarr_value);
    
    yvar_t key1 = YVAR_INT16(4444);
    yvar_t * value1 = ymap_get(map, key1);
    printf("got key %d: %d\n", key1.data.yint16_data, value1->data.yint16_data);
    
    yvar_t key2 = YVAR_CSTR("Oh?");
    yvar_t * value2 = ymap_get(map, key2);
    printf("key %s %s undefined\n", key2.data.ycstr_data.str, is_or_not[yvar_is_undefined(*value2)]);

    yvar_t key3 = YVAR_CSTR("Who?");
    yvar_t * value3 = ymap_get(map, key3);
    printf("key %s %s undefined\n", key3.data.ycstr_data.str, is_or_not[yvar_is_undefined(*value3)]);
    printf("got key %s: %s\n", key3.data.ycstr_data.str, value3->data.ycstr_data.str);

    {
        YMAP_FOREACH(map, key, value) {
            printf("key: ");

            switch (key->type) {
                case YVAR_TYPE_INT8:
                    printf("%d", key->data.yint8_data);
                    break;

                case YVAR_TYPE_INT16:
                    printf("%d", key->data.yint16_data);
                    break;
                case YVAR_TYPE_CSTR:
                    printf("%s", key->data.ycstr_data.str);
                    break;
                default:
                    printf("unknown data type\n");
                    break;
            }
            
            printf(" value: ");

            switch (value->type) {
                case YVAR_TYPE_INT8:
                    printf("%d", value->data.yint8_data);
                    break;

                case YVAR_TYPE_INT16:
                    printf("%d", value->data.yint16_data);
                    break;
                case YVAR_TYPE_CSTR:
                    printf("%s", value->data.ycstr_data.str);
                    break;
                default:
                    printf("unknown data type\n");
                    break;
            }
            
            printf("\n");
        }
    }
#endif
