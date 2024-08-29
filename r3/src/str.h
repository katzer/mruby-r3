#ifndef R3_STR_INTERN_H
#define R3_STR_INTERN_H

#ifdef __cplusplus
extern "C" {
#endif

void print_indent(int level);

#ifndef HAVE_STRDUP
char *strdup(const char *s);
#endif

#ifndef HAVE_STRNDUP
char *strndup(const char *s, long long unsigned int n);
#endif

#ifdef __cplusplus
}
#endif

#endif
