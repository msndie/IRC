#include <iostream>
#include <sys/socket.h>

void sendAll(const char *msg, unsigned long nbrOfBytes, int receiverFd) {
	unsigned long	sent;
	unsigned long	left;
	unsigned long	n;

	sent = 0;
	left = nbrOfBytes;
	std::cout << msg << std::endl;
	while (sent < nbrOfBytes) {
		n = send(receiverFd, msg + sent, left, 0);
		if (n == -1) {
			std::cerr << "Sending to " << receiverFd << " failed: "
					  << strerror(errno) << std::endl;
			break;
		}
		sent += n;
		left -= n;
	}
}
