#ifndef _YUKI_CONFIG_H_
#define _YUKI_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

#if (defined(__STDC_VERSION__))
# if (__STDC_VERSION__ == 199901L)
#  define YUKI_CONFIG_C99_ENABLED
# endif
#endif

#define YUKI_CONFIG_SECTION_YBUFFER "ybuffer"
#define YUKI_CONFIG_SECTION_YTABLE  "ytable"
#define YUKI_CONFIG_SECTION_YLOG    "ylog"

#ifdef __cplusplus
}
#endif

#endif
