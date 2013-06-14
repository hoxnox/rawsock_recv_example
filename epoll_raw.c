#include <nx_socket.h>

inline SOCKET mksock()
{
	SOCKET sock = socket(addr->sa_family, SOCK_RAW, IPPROTO_RAW);
	if(!IS_VALID_SOCK(sock))
	{
		ELOG << _("Error socket initialization.");
		return INVALID_SOCKET;
	}
	if(SetNonBlock(sock) < 0)
	{
		ELOG << _("Error switching socket to non-block mode.") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return INVALID_SOCKET;
	}
	if(SetReusable(sock) < 0)
	{
		ELOG << _("Error making socket reusable") << " "
			<< _("Message") << ": " << strerror(GET_LAST_SOCK_ERROR());
		close(sock);
		return INVALID_SOCKET;
	}
	return sock;
}

int main(int argc, char* argv[])
{
	SOCKET sock[2];
	sock[0] = mksock();
	sock[1] = mksock();

	int epollfd = epoll_create(1);
	if (epollfd == -1)
		error(1, errno, "Error epoll creating");
	epoll_event ev[2];
	ev[0].events = EPOLLIN | EPOLLPRI | EPOLLOUT;
	ev[0].data.fd = sock[0];
	ev[1].events = EPOLLIN | EPOLLPRI | EPOLLOUT;
	ev[1].data.fd = sock[1];

	return 0;
}
