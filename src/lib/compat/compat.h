
#ifndef OIN17_COMPAT_H
#define OIN17_COMPAT_H
/* ========================================================================== */

#include <stddef.h>

/* -------------------------------------------------------------------------- */

extern size_t strlcpy(char*, const char*, size_t);
extern void* reallocarray(void*, size_t, size_t);

/* ========================================================================== */
#endif /* !OIN17_COMPAT_H */
