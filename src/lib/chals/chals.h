/*
 * This file is part of oin17.
 */

#ifndef OIN17_CHALS_H
#define OIN17_CHALS_H
/* ========================================================================== */

#include "../priv.h"

/* -------------------------------------------------------------------------- */

#define UINT32SIZE	4294967295

/* TODO sync with cscoins/ca_config.txt */
#define NONCEMAX	(999999999 + 1)


/*
 * 2^64 - 1 : 18,446,744,073,709,551,615
 * strlen(2^64 - 1) : 6*3 + 2 = 20
 */
#define U64STRLEN	20

/* -------------------------------------------------------------------------- */

extern uint64_t getnonce(struct mt64*);
extern uint64_t mkseed(char*, uint64_t);

extern size_t mku64str(const uint64_t*, size_t, char*);

/* -------------------------------------------------------------------------- */

typedef int (*csolver_t)(const struct chal*, struct solver*);

extern int sortlist_resize(struct solver*, const struct chal*);
extern void sortlist_free(struct solver*);
extern int sortlist(const struct chal*, struct solver*);

extern int shortpath_resize(struct solver*, const struct chal*);
extern void shortpath_free(struct solver*);
extern int shortpath(const struct chal*, struct solver*);

/* ========================================================================== */
#endif /* !OIN17_CHALS_H */
