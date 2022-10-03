#include <iostream>
#include <sys/socket.h>

void sendAll(const char *msg, ssize_t nbrOfBytes, int receiverFd) {
	ssize_t	sent;
	ssize_t	left;
	ssize_t	n;

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
