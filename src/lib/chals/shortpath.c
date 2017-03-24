/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * Challenge solver: shortest path
 *
 *
 * Dijkstra search algorithm
 *
 * Path preference: down, up, right, left
 */

/* ========================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "chals.h"

#define NDIRS 4		/* number of directions we can move */

/* -------------------------------------------------------------------------- */

struct cell {
	int	x;
	int	y;
	int	c;		/* cost from src */

	struct cell*	p;	/* parent (always the shortest path) */

	/* queueing (next, prev) */
	struct cell*	qn;
	struct cell*	qp;
};

struct shortpath {
	size_t		max;	/* the maximum grid width we can solve */
	size_t		size;	/* the current challenge's grid width */
	char*		grid;	/* the grid */
	char*		sol;	/* solution str buffer */

	struct cell*	cells;	/* all cells */
	struct cell*	q;	/* the queue */
	struct cell*	src;	/* the src shorthand */
	struct cell*	dst;	/* the dst shorthand */
};

/* -------------------------------------------------------------------------- */

/*
 * The qadd() function is VITAL to the correctness of the
 * solution. It simulates python's heapq module, which not
 * only orders elements based on their priority, but also
 * on their value.
 *
 * Specifically, qadd() must order based on (1) priority,
 * (2) the value of y and (3) the value of x.
 * [Since the server uses (y, y) tupples].
 */

/* <0: a < b, 0: a=b, >0: a > b */
static int costcmp(const struct cell* a, const struct cell* b)
{
	if (a->c != b->c) {
		return (a->c - b->c);
	}
	if (a->y != b->y) {
		return (a->y - b->y);
	}
	return (a->x - b->x);
}

static void qadd(struct shortpath* sp, struct cell* cl)
{
	struct cell* p;

	if (0 == sp->q) {
		/* empty */
		sp->q = cl;
		cl->qp = 0;
		cl->qn = 0;
	} else if (0 < costcmp(sp->q, cl)) {
		/* the head costs more, replace */
		cl->qn = sp->q;
		cl->qp = 0;
		sp->q  = cl;
		cl->qn->qp = cl;
	} else {
		p = sp->q;
		while (0 != p->qn) {
			if (0 < costcmp(p->qn, cl)) {
				/* insertion p,cl,p->qn */
				cl->qn = p->qn;
				cl->qp = p;

				p->qn->qp = cl;
				p->qn     = cl;
				return;
			}
			p = p->qn;
		}
		/* tail */
		p->qn = cl;
		cl->qp = p;
		cl->qn = 0;
	}
}

static struct cell* qpop(struct shortpath* sp)
{
	struct cell* p;

	p = sp->q;
	if (0 != p) {
		sp->q = p->qn;
		p->qn = 0;
		if (0 != sp->q) {
			sp->q->qp = 0;
		}
	}

	return p;
}

static int qfind(const struct shortpath* sp, const struct cell* cl)
{
	if (cl == sp->q) {	/* cell is head */
		return 1;
	}

	/* cannot be !head and not have previous */
	return (0 != cl->qp);
}

static void qdel(struct shortpath* sp, struct cell* cl)
{
	struct cell* p;

	if (0 == qfind(sp, cl)) {
		return;
	}

	if (cl == sp->q) {
		sp->q = cl->qn;
		if (0 != sp->q) {
			sp->q->qp = 0;
		}
	} else if (0 != sp->q) {
		p = sp->q;
		while (0 != p->qn) {
			if (cl == p->qn) {
				p->qn = cl->qn;
				if (0 != p->qn) {
					p->qn->qp = p;
				}
				cl->qn = 0;
				cl->qp = 0;
				return;
			}
			p = p->qn;
		}
	}
}

static void prq(const struct shortpath* sp)
{
#if 0
	struct cell* p;

	p = sp->q;
	while (0 != p) {
		gprintf("(%u,%u)[%u]", p->x, p->y, p->c);
		p = p->qn;
	}
#else
	(void)sp;
#endif
}

/* -------------------------------------------------------------------------- */

#ifdef OIN17_DEBUG
static void dmpgrid(const struct shortpath* sp)
{
#if 0
	size_t x;
	size_t y;

	for (y = 0 ; sp->size > y ; y++) {
		for (x = 0 ; sp->size > x ; x++) {
			printf("%c", sp->grid[indexofxy(x, y, sp->size)]);
		}
		putchar('\n');
	}
#else
	(void)sp;
#endif
}
#endif

static size_t indexofxy(size_t x, size_t y, size_t width)
{
	return ((y * width) + x);
}

/* -------------------------------------------------------------------------- */

static struct cell*
getneigh(const struct shortpath* sp, const struct cell* cl, int dir)
{
	size_t x;
	size_t y;

	x = cl->x;
	y = cl->y;

	switch (dir) {
	case 0: /* down */
		y++;
		break;
	case 1: /* up */
		y--;
		break;
	case 2: /* right */
		x++;
		break;
	case 3: /* left */
		x--;
		break;
	default:
		eprintf("Internal error: unexpected direction in shortpath");
		exit(1);
	}

	if ('x' != sp->grid[indexofxy(x, y, sp->size)]) {
		return &sp->cells[indexofxy(x, y, sp->size)];
	}
	return 0;
}

static void search_init(struct shortpath* sp)
{
	size_t x;
	size_t y;
	size_t i;

	for (y = 0 ; sp->size > y ; y++) {
		for (x = 0 ; sp->size > x ; x++) {
			i = indexofxy(x, y, sp->size);
			sp->cells[i].x = x;
			sp->cells[i].y = y;
			sp->cells[i].c = -1;
			sp->cells[i].p = 0;
			sp->cells[i].qn = 0;
			sp->cells[i].qp = 0;
		}
	}

	sp->q = 0;
}

static int search(struct solver* slv)
{
	size_t i;
	struct cell* nei;
	struct cell* cur;
	struct shortpath* sp;

	sp = (struct shortpath*)slv->sp;
	search_init(sp);


	//gprintf("(%u,%u) -> (%u,%u)", sp->src->x, sp->src->y, sp->dst->x, sp->dst->y);


	sp->src->c = 0;
	qadd(sp, sp->src);

	while (0 != (cur = qpop(sp))) {
#if 0
		gprintf("cur=(%u, %u)", cur->x, cur->y);
		if ((sp->src != cur) && (sp->dst != cur)) {
			const char a[] = "0123456789abcdfghijklmnopqrtuvwyz";
			sp->grid[indexofxy(cur->x, cur->y, sp->size)] = a[cur->c % (sizeof(a) - 1)];
		}
#endif
		if (cur == sp->dst) {
			break;
		}

		for (i = 0 ; NDIRS > i ; i++) {
			if (0 == (nei = getneigh(sp, cur, i))) {
				/* cannot move there */
				continue;
			}

			if ((0 > nei->c) || ((cur->c + 1) < nei->c)) {
			//	gprintf("updating neighbor (%u,%u)", nei->x, nei->y);
				nei->c = (cur->c + 1);
				qdel(sp, nei);
				qadd(sp, nei);
				nei->p = cur;
			}
		}
		prq(sp);
	}

	if (cur != sp->dst) {
		return 0;
	}


#if 0
	struct cell* tmp = cur;
	while (sp->src != tmp) {
		if ((tmp != sp->dst) && (tmp != sp->src)) {
			sp->grid[indexofxy(tmp->x, tmp->y, sp->size)] = 'p';
		}
		tmp = tmp->p;
		if (0 == tmp) {
			gprintf("oops");
		}
	}
#endif

	/* use the queue to reverse the path */

	while (0 != qpop(sp)) {
		/* */;
	}
	i = (size_t)-1;
	while (sp->src != cur) {
		cur->c = i--;
		qadd(sp, cur);
		cur = cur->p;
	}
	cur->c = i--;
	qadd(sp, cur);

	i = 0;
	while (0 != (cur = qpop(sp))) {
		i += u64str(cur->y, &sp->sol[i]);
		i += u64str(cur->x, &sp->sol[i]);
		sp->sol[i] = '\0';
	}
	sp->sol[i] = '\0';

	return i;
}

/* -------------------------------------------------------------------------- */

static void clrgrid(struct shortpath* sp)
{
	size_t i;
	size_t m;

	memset(sp->grid, ' ', (sp->size * sp->size));

	m = (sp->size - 1);

	/* walls */
	for (i = 0 ; sp->size > i ; i++) {
		sp->grid[indexofxy(0, i, sp->size)] = 'x';	/* top */
		sp->grid[indexofxy(m, i, sp->size)] = 'x';	/* bottom */
		sp->grid[indexofxy(i, 0, sp->size)] = 'x';	/* left */
		sp->grid[indexofxy(i, m, sp->size)] = 'x';	/* right */
	}
}

static void popgrid(struct shortpath* sp, size_t nblks, struct mt64* mt)
{
	int x;
	int y;
	size_t i;

	clrgrid(sp);

	/* the start position */
	do {
		y = (mt64_rand(mt) % sp->size);
		x = (mt64_rand(mt) % sp->size);
		i = indexofxy(x, y, sp->size);
	} while (' ' != sp->grid[i]);
	sp->src = &sp->cells[i];
	sp->src->x = x;
	sp->src->y = y;
	sp->grid[i] = 's';

	/* the destination */
	do {
		y = (mt64_rand(mt) % sp->size);
		x = (mt64_rand(mt) % sp->size);
		i = indexofxy(x, y, sp->size);
	} while (' ' != sp->grid[i]);
	sp->dst = &sp->cells[i];
	sp->dst->x = x;
	sp->dst->y = y;
	sp->grid[i] = 'e';

	/* the blockers */
	for (i = 0 ; nblks > i ; i++) {
		y = (mt64_rand(mt) % sp->size);
		x = (mt64_rand(mt) % sp->size);

		if ( (sp->src->x == x) && (sp->src->y == y) ) {
			continue;
		}
		if ( (sp->dst->x == x) && (sp->dst->y == y) ) {
			continue;
		}
		/* we can safely add over an existing wall/block */
		sp->grid[indexofxy(x, y, sp->size)] = 'x';
	}
}

/* -------------------------------------------------------------------------- */

int shortpath(const struct chal* chl, struct solver* slv)
{
	size_t l;
	struct shortpath* sp;
	uint8_t d[SHA256_LEN];
	char lh[SHA256_DLEN + U64STRLEN + 1];


	sp = (struct shortpath*)slv->sp;

	/* FIXME don't do memcpy is chl->id is the same as before? */
	memcpy(lh, chl->lhash, SHA256_DLEN);
	sp->size = chl->params.sp.size;

#if 1
	/* generate the seed */
	slv->nonce = getnonce(slv->nmin, slv->nmax);
#else
	/* replicate server */
	slv->nonce = 837036405;
	memcpy(lh, "cf9574d5d9a95b568de5bd288c743a3afecc00575c39490d4a8a249c7ae81948", SHA256_DLEN);
#endif

	mt64_seed(slv->prng, mkseed(lh, slv->nonce));

	popgrid(sp, chl->params.sp.nblks, slv->prng);

	if (0 == (l = search(slv))) {
		return -1;
	}

	sha256(sp->sol, l, d);

	if (0 == memcmp(d, chl->pfix, chl->plen)) {
#ifdef OIN17_DEBUG
		dmpgrid(sp);
		char dgst[SHA256_DLEN + 1];
		bin2hex(d, sizeof(d), dgst);
		lh[SHA256_DLEN] = 0;
		gprintf("Found %llu for %s (last=%s)", slv->nonce, dgst, lh);
#endif
		return 0;
	}

	return -1;
}

/* -------------------------------------------------------------------------- */

int shortpath_resize(struct solver* slv, const struct chal* chl)
{
	void* tmp;
	size_t gs;
	struct shortpath* sp;

	if (0 == slv->sp) {
		if (0 == (slv->sp = calloc(1, sizeof(*sp)))) {
			return -1;
		}
	}

	sp = (struct shortpath*)slv->sp;
	if (sp->max >= chl->params.sp.size) {
		return 0;
	}

	gprintf("Growing shortpath workspace");
	gs = (chl->params.sp.size * chl->params.sp.size);
	tmp = REALLOC(sp->grid, gs, sizeof(char));
	if (0 == tmp) {
		eeprintf("Failed to resize shortpath grid");
		return -1;
	}
	sp->grid = tmp;

	tmp = REALLOC(sp->sol, gs, 1);
	if (0 == tmp) {
		eeprintf("Failed to resize shortpath solution");
		return -1;
	}
	sp->sol = tmp;

	tmp = REALLOC(sp->cells, gs, sizeof(struct cell));
	if (0 == tmp) {
		eeprintf("Failed to resize shortpath cells");
		return -1;
	}
	sp->cells = tmp;

	sp->max = chl->params.sp.size;

	return 0;
}

void shortpath_free(struct solver* slv)
{
	struct shortpath* sp;

	if (0 != slv->sp) {
		sp = (struct shortpath*)slv->sp;
		if (0 != sp->grid) {
			free(sp->grid);
		}
		if (0 != sp->sol) {
			free(sp->sol);
		}
		if (0 != sp->cells) {
			free(sp->cells);
		}
		free(sp);
		slv->sp = 0;
	}
}

/* ========================================================================== */
