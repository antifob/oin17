
#include "cmd.h"

int ca_recv(int sk, void* buf, size_t len)
{
	return ws_recv(sk, buf, len);
}
