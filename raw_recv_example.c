#include <nx_socket.h>
#include <sys/epoll.h>
#include <error.h>
#include <memory.h>
#include <stdio.h>

inline SOCKET mksock()
{
	/*Initialize RAW socket UDP level*/
	SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);

	/*setup some more settings for polling*/
	if(!IS_VALID_SOCK(sock))
		error(1, 0, "Error socket initialization");
	if(SetNonBlock(sock) < 0)
		error(1, errno, "Error switching socket to non-block mode.");
	if(SetReusable(sock) < 0)
		error(1, errno, "Error making socket reusable");

	return sock;
}

int main(int argc, char* argv[])
{
	const char my_ip[] = "10.101.0.15";
	const char udpecho_ip[] = "10.101.0.16";

	SOCKET sock[2];
	char buf[] = "Hello, world!";
	sock[0] = mksock();
	sock[1] = mksock();

	int epollfd = epoll_create(1);
	if (epollfd == -1)
		error(1, errno, "Error epoll creating");
	struct epoll_event ev[2];
	ev[0].events = EPOLLIN | EPOLLPRI;
	ev[0].data.fd = sock[0];
	ev[1].events = EPOLLIN | EPOLLPRI;
	ev[1].data.fd = sock[1];
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock[0], &ev[0]) == -1)
		error(1, errno, "Error adding event 1 to epoll");
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, sock[1], &ev[1]) == -1)
		error(1, errno, "Error adding event 2 to epoll");

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(udpecho_ip);
	addr.sin_port = htons(5100);
	struct sockaddr_in self;
	memset(&self, 0, sizeof(struct sockaddr_in));
	self.sin_family = AF_INET;
	self.sin_addr.s_addr = inet_addr(my_ip);
	self.sin_port = htons(5101);
	
	if (RawSendTo(sock[0], buf, sizeof(buf), 0,
	              (struct sockaddr*)&self,
	              (struct sockaddr*)&addr,
	              sizeof(struct sockaddr_in)) == 0)
	{
		error(1, errno, "Error RawSendTo data.");
	}
	struct epoll_event events[2];
	memset(events, 0, sizeof(struct epoll_event)*2);
	while(1)
	{
		size_t nfds = epoll_wait(epollfd, events, 2 + 1, 1000);
		size_t i = 0;
		if (nfds == -1)
			error(1, errno, "Error calling epoll");
		for (i = 0; i < nfds; ++i)
		{
			if ((events[i].events & EPOLLIN) == EPOLLIN ||
			   (events[i].events & EPOLLPRI) == EPOLLPRI)
			{
				struct sockaddr_storage tmp_addr;
				socklen_t len = sizeof(tmp_addr);
				char rbuf[65355];
				memset(rbuf, 0, sizeof(buf));
				int rs = recvfrom(events[i].data.fd, rbuf, sizeof(rbuf), 0,
						(struct sockaddr*)&tmp_addr, &len);
				if(rs < 0)
					error(1, errno, "Error receiving data from socket.");
				char tmp[50];
				if(inet_ntop(tmp_addr.ss_family, GetAddr((struct sockaddr*)&tmp_addr),
							tmp, sizeof(tmp)) == NULL)
				{
					error(1, errno, "Error converting addr to string.");
				}
				printf("Socket %lu (from %s): %s\n", i, tmp, rbuf + 28);
			}
		}
		printf("heartbeat\n");
		fflush(stdout);
	}

	return 0;
}

