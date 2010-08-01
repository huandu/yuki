#ifndef _YUKI_STRING_H_
#define _YUKI_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YCSTR(s) {.size = sizeof((s)) - 1, .str = s}

#ifdef __cplusplus
}
#endif

#endif
