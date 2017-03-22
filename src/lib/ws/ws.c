/*
 * This file is part of oin17.
 *
 * Copyright 2017, Philippe Grégoire
 *
 * libwebsockets abstraction
 *
 * Due to the braindead API of (all) WebSockets libraries,
 * we hide their ugliness behind a threaded translator.
 */

#if 0
The module allows the user to maintain control of the execution by
returning an interfacing socket and doing I/O in a thread. Whenever
the user writes data to its socket, the thread reads it and forwards
it to the remote node. When data arrives from the node, it forwards
it back to the user by writing to the other side of the socket. This
way, the application can read, write and poll the socket as if the
WebSockets protocol was not involved.
#endif

/* TODO TLS, see callback event 58 */
/* TODO TLS, ciphers */

/* ========================================================================== */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <libwebsockets.h>

#include "priv.h"

/* -------------------------------------------------------------------------- */

#define MAXUCONNS	16

#define S_SENDREQ	0x01	/* there is data to send */

struct uconn {
	int		id;		/* the uconn's id */
	int		uq;		/* the uqueue on which to send event */
	struct lws*	ws;		/* libws connection */

	int		st;		/* state */
	uint8_t		fb[MAXMSGLEN];	/* the fragments buffer */
	size_t		fo;		/* the fragment's current offset */
	struct mbuf*	iq;		/* the incoming message queue */
	struct mbuf*	oq;		/* the outgoing message queue */
};

static struct lws_context*		ctx;
static struct lws_client_connect_info	ccin;

static pthread_mutex_t	lock = PTHREAD_MUTEX_INITIALIZER;
static struct uconn	uconns[MAXUCONNS];

/* -------------------------------------------------------------------------- */

static int uconn_isvalid(int id)
{
	if ((0 > id) || (MAXUCONNS <= id)) {
		return 0;
	}
	return (id == uconns[id].id);
}

static struct uconn* newuconn(int uq)
{
	size_t i;

	for (i = 0 ; MAXUCONNS > i ; i++) {
		if ((0 == uconns[i].ws) && (0 > uconns[i].id)) {
			uconns[i].id = i;
			uconns[i].uq = uq;
			uconns[i].ws = 0;
			uconns[i].iq = 0;
			uconns[i].oq = 0;
			uconns[i].st = 0;
			uconns[i].fo = 0;
			return &uconns[i];
		}
	}

	gprintf("no more uconns");
	return 0;
}

#define deluconn(p)	((p)->id = -1)

/* -------------------------------------------------------------------------- */

#if defined(OIN17_DEBUG)
static void prwsevent(int id)
{
	//return;
	printf("Got event %i: ", id);
	switch (id) {
	case LWS_CALLBACK_ESTABLISHED:
		printf("callback established\n");
		break;
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR:
		printf("client connection error\n");
		break;
	case LWS_CALLBACK_CLOSED:
		printf("connection closed\n");
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED: /* 3 */
		printf("connexion established\n");
		break;
	case LWS_CALLBACK_CLIENT_WRITEABLE: /* 10 */
		printf("writeable\n");
		break;
	case LWS_CALLBACK_CLOSED_CLIENT_HTTP: /* 45 */
		printf("closed\n");
		break;
	case LWS_CALLBACK_CLIENT_RECEIVE: /* 8 */
		break;
	case LWS_CALLBACK_PROTOCOL_INIT: /* 27 */
		printf("protocol init\n");
		break;
	case LWS_CALLBACK_WSI_DESTROY: /* 30 */
		printf("wsi destroy\n");
		break;
	case LWS_CALLBACK_HTTP_DROP_PROTOCOL: /* 50 */
		printf("http drop\n");
		break;
	//case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: /* 58 */
		//printf("server cert verification\n");
		//break;

	/* ignored */
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: /* 24 */
	case LWS_CALLBACK_WSI_CREATE:
	case LWS_CALLBACK_GET_THREAD_ID: /* 31 */
	case LWS_CALLBACK_LOCK_POLL:
	case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: /* 2 */
	case LWS_CALLBACK_UNLOCK_POLL:
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
	case LWS_CALLBACK_ADD_POLL_FD:
	case LWS_CALLBACK_DEL_POLL_FD:
		putchar('\n');
		break;

	default:
		printf("(unknown)\n");
		break;
	}
}
#else
# define prwsevent(o)
#endif

/* -------------------------------------------------------------------------- */

static void reportconn(const struct uconn* uc)
{
	struct uevent ue;

	gprintf("Reporting connection established");


	UQ_SET(&ue, uc->id, UQFILT_WSOCK, 0,
	       (QNOTE_TRIGGER | QNOTE_FFOR | QNOTE_CONN), 0, 0);

	uevent(uc->uq, &ue, 1, 0, 0, 0);
}

static void reportclose(const struct uconn* uc)
{
	struct uevent ue;

	gprintf("Reporting close event");


	UQ_SET(&ue, uc->id, UQFILT_WSOCK, 0,
	       (QNOTE_TRIGGER | QNOTE_FFCOPY | QNOTE_CLOSED), 0, 0);

	uevent(uc->uq, &ue, 1, 0, 0, 0);
}

/* queue message */
static int __recvfrag(struct uconn* uc)
{
	struct mbuf* mb;

	/* messages are always ascii, facilitate further manipulation */
	uc->fb[uc->fo] = '\0';
	uc->fo++;

	gprintf("Enqueuing fragged message (%zu)", uc->fo);
	gprintf("q:'%s'", uc->fb);
	

	if (0 != (mb = mbuf(uc->fb, uc->fo))) {
		pthread_mutex_lock(&lock);
		mbufq(&uc->iq, mb);
		uc->fo = 0;
		pthread_mutex_unlock(&lock);
	}

	return (0 == mb) ? -1 : 0;
}

/*
 * libws is dumb enough to relay fragmented messages even though they
 * were not. In order for us to relay _full_ messages to the user,
 * buffer the fragments until the message is fully received.
 */
static int
recvfrag(struct lws* ws, const void* buf, size_t len, struct uconn* uc)
{
	size_t r;
	size_t t;
	struct uevent ue;

	gprintf("Received a fragment");


	r = lws_remaining_packet_payload(ws);
	t = (uc->fo + len + r);
	if (sizeof(uc->fb) < t) {
		eprintf("Message too large for fragment buffer: %zu", t);
		return -1;
	}

	memcpy(&uc->fb[uc->fo], buf, len);
	uc->fo += len;

	if (0 != r) {
		return 0;
	}

	if (0 != __recvfrag(uc)) {
		eeprintf("Failed to queue incoming message");
		return -1;
	}

	/* Report the event. Ignore errors. */
	UQ_SET(&ue, uc->id, UQFILT_WSOCK, 0,
	       (QNOTE_TRIGGER | QNOTE_FFOR | QNOTE_READ), 0, 0);
	uevent(uc->uq, &ue, 1, 0, 0, 0);

	return 0;
}

/* relay one message to the ca */
static int relayca(struct uconn* uc, struct lws* ws)
{
	int n;
	int r;
	unsigned char b[1024]; /* arbitrary, should fit all */

	gprintf("Relaying message to CA");


	/*
	 * libws requires LWS_PRE bytes at the
	 * beginning of the buffer. We could
	 * hack mbufp so that the data is stored
	 * at the correct position, but we play
	 * save and use memmove() instead.
	 */
	if (0 > (n = mbufp(&uc->oq, b, sizeof(b)))) {
		eeprintf("Failed to pop message for CA");
		return -1;
	}
	memmove(&b[LWS_PRE], b, n);

	if (0 != n) {
		/* WTF pass the data address!? */
		if (n != (r = lws_write(ws, &b[LWS_PRE], n, LWS_WRITE_TEXT))) {
			eeprintf("Failed to write full message to CA (%i, %i)", r, n);
			return -1;
		}
	}

	return 0;
}

static int
lws_handler(struct lws* ws, enum lws_callback_reasons rs,
            void* user, void* in, size_t len)
{
	char b[1024];
	struct uconn* uc;

	prwsevent(rs);

	uc = (struct uconn*)user;


	switch (rs) {
	case LWS_CALLBACK_CLIENT_CONNECTION_ERROR: /* 1 */
		if (0 != in) {
			eprintf("Connection error: %s", in);
		} else {
			eprintf("Unknown connection error");
		}
		reportclose(uc);
		break;
	case LWS_CALLBACK_CLOSED:
		reportclose(uc);
		break;
	case LWS_CALLBACK_CLIENT_ESTABLISHED: /* 3 */
		/* TODO lock? */
		if (0 != (S_SENDREQ & uc->st)) {
			lws_callback_on_writable(ws);
			uc->st &= ~S_SENDREQ;
		}
		reportconn(uc);
		break;

	case LWS_CALLBACK_CLIENT_RECEIVE: /* 8 */
		return recvfrag(ws, in, len, uc);

	case LWS_CALLBACK_CLIENT_WRITEABLE: /* 10 */
		return relayca(uc, ws);

	case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS: /* 21 */
		wprintf("ssl");
		break;

	case LWS_CALLBACK_WSI_DESTROY: /* 30 */
		while (0 != mbufp(&uc->iq, b, sizeof(b))) {
			/**/;
		}
		while (0 != mbufp(&uc->iq, b, sizeof(b))) {
			/**/;
		}
		uc->ws = 0;	/* release after everything else */
		return 0;

	case LWS_CALLBACK_WS_PEER_INITIATED_CLOSE: /* 38 */
		wprintf("Server initiated disconnexion");
		return -1;

#if 1
	case LWS_CALLBACK_OPENSSL_PERFORM_SERVER_CERT_VERIFICATION: /* 58 */
		wprintf("tls callback");
		/* return 0=accept, 1=reject */
		/* user=X509_STORE_CTX, in=SSL */
		//return (0 == checkcert(user, in)) ? 0 : 1;
		break;
#endif


	/* IGNORED */
	//case LWS_CALLBACK_OPENSSL_LOAD_EXTRA_CLIENT_VERIFY_CERTS:
	case LWS_CALLBACK_CLIENT_FILTER_PRE_ESTABLISH: /* 2 */
	case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER: /* 24 */
	case LWS_CALLBACK_PROTOCOL_INIT: /* 27 */
	case LWS_CALLBACK_WSI_CREATE: /* 29 */
	case LWS_CALLBACK_GET_THREAD_ID: /* 31 */
	case LWS_CALLBACK_ADD_POLL_FD: /* 32 */
	case LWS_CALLBACK_DEL_POLL_FD: /* 33 */
	case LWS_CALLBACK_CHANGE_MODE_POLL_FD: /* 34 */
	case LWS_CALLBACK_LOCK_POLL: /* 35 */
	case LWS_CALLBACK_UNLOCK_POLL: /* 36 */
	case LWS_CALLBACK_CLOSED_CLIENT_HTTP: /* 45 */
	case LWS_CALLBACK_HTTP_DROP_PROTOCOL: /* 50 */
		break;
	default:
		wprintf("unhandled libws event: %i", rs);
		break;
	}

	return 0;
}

void ws_dispatch(void* _)
{
	(void)_;
	lws_service(ctx, 100);
}

/* -------------------------------------------------------------------------- */

int ws_connect(int uq)
{
	struct uconn* uc;
	struct lws_client_connect_info ci;


	if (0 == (uc = newuconn(uq))) {
		return -1;
	}

	memcpy(&ci, &ccin, sizeof(ci));
	ci.userdata = uc;

	if (0 == (uc->ws = lws_client_connect_via_info(&ci))) {
		deluconn(uc);
		return -1;
	}

	return uc->id;
}

/*
 * WARNING
 *
 * Closing the libws connexion requires that we return non-zero from the
 * callbacks. Since we cannot explicitly indicate a close (WTF?!), we
 * reset the uconn marker (which is going to be caught by the callback
 * routine (hopefully)) to indicate a closed connexion.
 *
 * In other words, libws will keep handling untracked data until the
 * callback drops the connexion dead. From our perspective, we can reuse
 * closed-but-dangling uconns since they won't be reused anyways.
 */
void ws_close(int id)
{
	if (0 == uconn_isvalid(id)) {
		errno = EINVAL;
		return;
	}

	pthread_mutex_lock(&lock);
	if (id == uconns[id].id) {
		deluconn(&uconns[id]);
	}
	pthread_mutex_unlock(&lock);
}

int ws_send(int id, const void* buf, size_t len)
{
	int r;
	struct mbuf* m;


	if (0 == uconn_isvalid(id)) {
		errno = EINVAL;
		return -1;
	}


	if (0 == (m = mbuf(buf, len))) {
		return -1;
	}

	r = -1;
	pthread_mutex_lock(&lock);
	if (id == uconns[id].id) {
		gprintf("Queueing outgoing message");
		mbufq(&uconns[id].oq, m);
		r = len;

		/*
		 * If the connexion was not established yet,
		 * libws won't accept outgoing data and won't
		 * set its WRITEABLE mark.
		 * S_SENDREQ indicates to the handler that
		 * we already have outgoing data and that it
		 * must call *_on_writable().
		 * In other contexts, it has purpose and
		 * the below *_on_writable() will make the job.
		 */
		uconns[id].st |= S_SENDREQ;
		lws_callback_on_writable(uconns[id].ws);
	}
	pthread_mutex_unlock(&lock);

	return r;
}

int ws_recv(int id, void* buf, size_t len)
{
	int r;

	if (0 == uconn_isvalid(id)) {
		errno = EINVAL;
		return -1;
	}

	r = -1;
	pthread_mutex_lock(&lock);
	if (id == uconns[id].id) {
		r = mbufp(&uconns[id].iq, buf, len);
	}
	pthread_mutex_unlock(&lock);

	return r;
}

/* -------------------------------------------------------------------------- */

// FIXME rx_buffer_size (decreases fragments)
static const struct lws_protocols lws_prots[] = {
	{ .name				= "dumb-increment-protocol",
	  .callback			= lws_handler,
	  .per_session_data_size	= 0,
	  .rx_buffer_size		= 512
	},
	{ .name = 0 }
};

#if 0
// FIXME support gzipped data?
static const struct lws_extension lws_exts[] = {
	{ 0, 0, 0 }
};
#endif

static int __init_ctx(void)
{
	struct lws_context_creation_info i;

	memset(&i, 0, sizeof(i));

	i.port = CONTEXT_PORT_NO_LISTEN;
	i.gid  = -1;
	i.uid  = -1;

	i.ssl_ca_filepath = "/etc/ssl/certs/ca-certificates.crt";
	i.protocols  = lws_prots;
	i.options   |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
	//i.extensions = lws_exts;

	ctx = lws_create_context(&i);

	return (0 == ctx) ? -1 : 0;
}

/*
 * libws is b-d.
 *
 * Connexion infos must be stored in a static buffer.
 */
static char __uri[256];	/* FIXME */
static char __path[16];	/* "/client\0" */

/* prepare the connexion info for future use */
static int __init_ccin(const char* uri)
{
	char* p;
	char* s;

	memset(&ccin, 0, sizeof(ccin));

	/* save the uri for future references */
	if (sizeof(__uri) <= strlcpy(__uri, uri, sizeof(__uri))) {
		eprintf("[ws] uri is too large: %s > %i", uri, sizeof(__uri));
		return -1;
	}

	if (0 != lws_parse_uri(__uri, (const char**)&s, &ccin.address, &ccin.port, (const char**)&p)) {
		eprintf("[ws] cannot parse uri: %s", uri);
		return -1;
	}
	/* WTF Because libws treats the first '/' as part of the hostname */
	strcpy(&__path[1], p);
	__path[0] = '/';

	if ((0 == strncmp("wss", s, 3)) || (0 == strncmp("https", s, 5))) {
		ccin.ssl_connection = LCCSCF_USE_SSL;
	}
	ccin.context = ctx;
	ccin.host    = ccin.address;
	ccin.origin  = ccin.address;
	ccin.path    = __path;
	ccin.protocol = lws_prots[0].name;

	gprintf("CA Server set to %s:%i%s", ccin.address, ccin.port, ccin.path);

	return 0;
}

static void __init_uconns(void)
{
	size_t i;

	for (i = 0 ; MAXUCONNS > i ; i++) {
		uconns[i].id = -1;
		uconns[i].ws = 0;
	}
}

int init_ws(const char* uri)
{
	lws_set_log_level(0, 0);

	if (0 != __init_ctx()) {
		return -1;
	}
	if (0 != __init_ccin(uri)) {
		return -1;
	}

	__init_uconns();

	return 0;
}

void exit_ws(void)
{
	if (0 != ctx) {
		lws_context_destroy(ctx);
		ctx  = 0;
	}
}

/* ========================================================================== */
