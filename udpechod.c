/* @author $username$ <$usermail$>
 * @date $date$
 *
 * Simplest UDP echo server*/

#include <errno.h>
#include <stdio.h>
#include <memory.h>
#include <malloc.h>

#include <nx_socket.h>

int main(int argc, char * argv[])
{
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (!sock)
		error(1, errno, "Error calling socket");
	int on = 1, off = 0;
	if(setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)))
		error(1, errno, "Error setting reusable");
	struct sockaddr_in self;
	memset(&self, 0, sizeof(self));
	self.sin_family = AF_INET;
	self.sin_addr.s_addr = inet_addr("127.0.0.1");
	self.sin_port = htons(5100);
	if (bind(sock, (struct sockaddr*)&self, sizeof(self)) < 0)
		error(1, errno, "Error calling bind");
	char buf[0x10000];
	memset(buf, 0, sizeof(buf));
	while(1)
	{
		struct sockaddr_in addr;
		memset(&addr, 0, sizeof(addr));
		socklen_t addrln = sizeof(addr);
		ssize_t rs = recvfrom(sock, buf, sizeof(buf), 0,
				(struct sockaddr*)&addr, &addrln);
		if(rs == -1)
			error(1, errno, "Error calling recvfrom");
		rs = sendto(sock, buf, rs, 0,
				(struct sockaddr*)&addr, sizeof(addr));
		if(rs == -1)
			error(1, errno, "Error calling sendto");
		fflush(stdout);
	}
	return 0;
}

