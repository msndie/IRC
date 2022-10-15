#include <fstream>
#include "../includes/Server.hpp"

void	Server::setupStruct() {
	struct addrinfo	hints = {};
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
	int	yes;

	yes = 1;
	_socketFd = socket(_serverInfo->ai_family, _serverInfo->ai_socktype, _serverInfo->ai_protocol);
	if (_socketFd == -1) {
		_err.append("An error on socket: ");
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

void	Server::bindSocketToPort() {
	if (bind(_socketFd, _serverInfo->ai_addr, _serverInfo->ai_addrlen) == -1) {
		close(_socketFd);
		freeaddrinfo(_serverInfo);
		_err.append("An error on bind: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

void	Server::startListening() {
	if (listen(_socketFd, BACKLOG) == -1) {
		close(_socketFd);
		_err.append("An error on listen: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

void	Server::initPollFdSet() {
	_pollFds = static_cast<pollfd *>(malloc(sizeof(struct pollfd) * _fdPollSize));
	if (_pollFds == nullptr) {
		close(_socketFd);
		throw LaunchFailed("Malloc error");
	}
	_pollFds[0].fd = _socketFd;
	_pollFds[0].events = POLLIN;
	for (int i = 1; i < _fdPollSize; ++i) {
		_pollFds[i].fd = -1;
	}
}

void	Server::prepareMotd() {
	std::ifstream	ifs;
	std::string		line;
	std::string		templ;

//	ifs.open("resources/message.motd", std::ifstream::in);
	ifs.open("../resources/message.motd", std::ifstream::in);
	if (!ifs.is_open()) {
		return;
	}
	while (getline(ifs, line)) {
		_motd.push_back(line);
	}
	ifs.close();
}

void	Server::configureServer() {
	_configuration = new Configuration();
//	_configuration->parseConfiguration("resources/configuration.yaml");
	_configuration->parseConfiguration("../resources/configuration.yaml");
	std::map<std::string, std::string>::const_iterator connections;
	std::map<std::string, std::string>::const_iterator timeOut;
	int tmp;

	_name = _configuration->getConfig().at("server.name");
	connections = _configuration->getConfig().find("server.connections");
	timeOut = _configuration->getConfig().find("server.registration.timeout");
	if (connections == _configuration->getConfig().end()
		|| (tmp = std::atoi(connections->second.c_str())) < 5 || tmp > 100) {
		std::cout << "Using default max number of connections" << std::endl;
	} else {
		std::cout << "Max number of connections is " << tmp << std::endl;
		_maxNbrOfConnections = tmp;
	}
	if (timeOut == _configuration->getConfig().end()
		|| (tmp = std::atoi(timeOut->second.c_str())) < 10 || tmp > 180) {
		std::cout << "Using default registration time out" << std::endl;
	} else {
		std::cout << "Registration time out is " << tmp << std::endl;
		_registrationTimeOut = tmp;
	}
}
