#include "Server.hpp"

Server::Server(const char* port) : _serverInfo(nullptr), _port(port), _socketFd(-1),
									_pollFds(nullptr), _fdCount(0), _fdPollSize(5) {}

Server::~Server() {}

// Load info in addrinfo struct
void	Server::setupStruct() {
	struct addrinfo	hints;
	int 			status;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	status = getaddrinfo(nullptr, _port, &hints, &_serverInfo);
	if (status != 0) {
		std::string err;
		err.append("getaddrinfo error: ");
		err.append(gai_strerror(status));
		throw LaunchFailed(err.c_str());
	}
}

// Get us a socket, set it to non block mode and reuse it if it's been in use
void	Server::createSocket() {
	int	yes;

	yes = 1;
	_socketFd = socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol);
	if (_socketFd == -1) {
		_err.append("An error on socket: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}

	// Do i need this?
	/*
	if (fcntl(_socketFd, F_SETFL, O_NONBLOCK) == -1) {
		_err.append("An error on fcntl: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}
	 */

	if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
		close(_socketFd);
		_err.append("An error on setsockopt: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}
}

// Bind out socket to port
void Server::bindSocketToPort() {
	if (bind(_socketFd, _serverInfo->ai_addr, _serverInfo->ai_addrlen) == -1) {
		close(_socketFd);
		freeaddrinfo(_serverInfo);
		_err.append("An error on bind: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

// Start listening on port for incoming connections
void Server::startListening() {
	if (listen(_socketFd, BACKLOG) == -1) {
		close(_socketFd);
		_err.append("An error on listen: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

// Add new file descriptor to the set
void Server::addToPollSet(int inFd) {
	if (_fdCount == _fdPollSize) {
		_fdPollSize *= 2;
		_pollFds = static_cast<pollfd *>(reallocf(_pollFds,
												 (sizeof(struct pollfd) *
												  _fdPollSize)));
		if (_pollFds == nullptr) {
			close(_socketFd);
			throw RuntimeServerError("Malloc error");
		}
	}
	_pollFds[_fdCount].fd = inFd;
	_pollFds[_fdCount].events = POLLIN;
	_fdCount++;
}

void Server::deleteFromPollSet(int i) {
	_pollFds[i] = _pollFds[--_fdCount];
}

// The only public method
void Server::startServer() {
	int status;
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int inFd;
	char	buf[512];

	if (!_err.empty()) {
		_err.clear();
	}

//	Delegated private methods, almost all of them can throw exception
	setupStruct();
	createSocket();
	bindSocketToPort();
	freeaddrinfo(_serverInfo);
	startListening();

//	Allocating our poll set
	_pollFds = static_cast<pollfd *>(malloc(sizeof(struct pollfd) * _fdPollSize));
	if (_pollFds == nullptr) {
		close(_socketFd);
		throw LaunchFailed("Malloc error");
	}

//	Add our server socket to poll set
	_pollFds[0].fd = _socketFd;
	_pollFds[0].events = POLLIN;
	_fdCount++;

	std::cout << "Server waiting for connection" << std::endl;

	while (true) { // Main loop
		int pollCount = poll(_pollFds, _fdCount, -1); // Will wait forever for event

		if (pollCount == -1) { // Poll error
			_err.append("An error on poll: ");
			_err.append(strerror(errno));
			throw RuntimeServerError(_err.c_str());
		}

		for (int i = 0; i < _fdCount; i++) { // Run through existing connections

			if (_pollFds[i].revents & POLLIN) { // Check if someone ready to read

//				If out server socket ready to read, we add new connection to the poll set
				if (_pollFds[i].fd == _socketFd) {
					addrLen = sizeof remoteAddr;
					inFd = accept(_socketFd, reinterpret_cast<sockaddr*>(&remoteAddr), &addrLen);
					if (inFd == -1) {
						std::cerr << "Accept error: " << strerror(errno) << std::endl;
					} else {
						addToPollSet(inFd);
						std::cout << "NEW CONNECTION: fd " << inFd - 1 << std::endl;
					}
				} else { // Client send message
					int senderFd = _pollFds[i].fd;
					ssize_t bytes = recv(senderFd, buf, sizeof buf, 0);
					if (bytes <= 0) { // Get error or connection closed by client
						if (bytes == 0) {
							std::cout << "Client with fd " << senderFd
										<< " close connection" << std::endl;
						} else {
							std::cerr << "Recv error: " << strerror(errno)
										<< std::endl;
						}
						close(senderFd);
						deleteFromPollSet(i);
					} else { // Got message from client
						// Send to every one
						for (int j = 0; j < _fdCount; j++) {
							int destFd = _pollFds[j].fd;
							if (destFd != _socketFd && destFd != senderFd) {
								if (send(destFd, buf, bytes, 0) == -1) {
									std::cerr << "Send error: " << strerror(errno)
											  << std::endl;
								}
							}
						}
					}
				}
			}
		}
	}
}

const char *Server::LaunchFailed::what() const throw() {
	return msg;
}

const char *Server::RuntimeServerError::what() const throw() {
	return msg;
}
