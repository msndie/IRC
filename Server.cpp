#include "Server.hpp"

Server::Server(const char* port) : _port(port) {}

Server::~Server() {}

void	Server::setupStructs() {
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

void	Server::createSocket() {
	int			yes;

	yes = 1;
	_socketFd = socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol);
	if (_socketFd == -1) {
		_err.append("An error on socket: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}

	if (fcntl(_socketFd, F_SETFL, O_NONBLOCK) == -1) {
		_err.append("An error on fcntl: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}

	if (setsockopt(_socketFd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1) {
		close(_socketFd);
		_err.append("An error on setsockopt: ");
		_err.append(strerror(errno));
		freeaddrinfo(_serverInfo);
		throw LaunchFailed(_err.c_str());
	}
}

void Server::startServer() {
	int status;
	struct sockaddr_storage their_addr;
	socklen_t addr_size;
	int in_fd;

	setupStructs();
	createSocket();

	status = bind(_socketFd, _serverInfo->ai_addr, _serverInfo->ai_addrlen);
	if (status == -1) {
		close(_socketFd);
		freeaddrinfo(_serverInfo);
		_err.append("An error on bind: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}

	freeaddrinfo(_serverInfo);

	status = listen(_socketFd, BACKLOG);
	if (status == -1) {
		close(_socketFd);
		_err.append("An error on listen: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}

	std::cout << "Server waiting for connection" << std::endl;

	addr_size = sizeof their_addr;
	in_fd = accept(_socketFd, (struct sockaddr*)&their_addr, &addr_size);
	if (in_fd == -1) {
		close(_socketFd);
		_err.append("An error on accept: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
	close(_socketFd);

	status = send(in_fd, "Kek", strlen("Kek"), 0);
//	while(status < strlen("Kek") && status != -1) {
//		TODO
//	}
	if (status == -1) {
		close(_socketFd);
		_err.append("An error on send: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

const char *Server::LaunchFailed::what() const throw() {
	return msg;
}
