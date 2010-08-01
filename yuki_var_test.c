#include <stdio.h>

#include "yuki.h"

int main()
{
    yvar_t yvar = YVAR_INT8(11);
    yvar_t pyvar[] = {YVAR_INT16(2222)};
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

    getchar();
    return 0;
}
