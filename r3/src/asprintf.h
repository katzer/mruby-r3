#ifndef ASPRINTF_H
#define ASPRINTF_H

#ifdef __cplusplus
extern "C" {
#endif

#if defined(__GNUC__) && ! defined(_GNU_SOURCE)
# define _GNU_SOURCE
# include <stdio.h>
#elif defined(_MSC_VER)
int asprintf(char **strp, const char *format, ...);
#endif

#ifdef __cplusplus
}
#endif

#endif // ASPRINTF_H
