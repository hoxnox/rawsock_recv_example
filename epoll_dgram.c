#include <nx_socket.h>

int main(int argc, char * argv[])
{
	SOCKET s[100];
	int i;

	for(i = 0;  i < 100; ++i)
		s[i] = socket(AF_INET, SOCK_DGRAM, 0);
	while(true)
	{
	}
}
