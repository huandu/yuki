#ifndef _YUKI_STRING_H_
#define _YUKI_STRING_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YCSTR(s) {sizeof((s)) - 1, s}
#define YCSTR_WITH_SIZE(s, len) {(len), s}

#ifdef __cplusplus
}
#endif

#endif
