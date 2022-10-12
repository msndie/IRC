#include <fstream>
#include "Server.hpp"

Server::Server(const char* port, const char* password) : _port(port), _pass(password) {
	_serverInfo = nullptr;
	_pollFds = nullptr;
	_configuration = nullptr;
	_socketFd = -1;
	_fdCount = 0;
	_fdPollSize = 5;
}

Server::~Server() {}

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

void Server::bindSocketToPort() {
	if (bind(_socketFd, _serverInfo->ai_addr, _serverInfo->ai_addrlen) == -1) {
		close(_socketFd);
		freeaddrinfo(_serverInfo);
		_err.append("An error on bind: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

void Server::startListening() {
	if (listen(_socketFd, BACKLOG) == -1) {
		close(_socketFd);
		_err.append("An error on listen: ");
		_err.append(strerror(errno));
		throw LaunchFailed(_err.c_str());
	}
}

void Server::initPollFdSet() {
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

void Server::deleteChannel(Channel *channel) {
	_channels.erase(channel->getName());
	delete channel;
}

void Server::prepareMotd() {
	std::ifstream	ifs;
	std::string		line;
	std::string		templ;

	ifs.open("../resources/message.motd", std::ifstream::in);
	if (!ifs.is_open()) {
		return;
	}
	while (getline(ifs, line)) {
		_motd.push_back(line);
	}
	ifs.close();
}

void Server::concatMsgs(std::string &rpl, const std::vector<std::string> &params, int start) {
	std::vector<std::string>::const_iterator iter;

	iter = params.begin();
	while (start != 0) {
		++iter;
		--start;
	}
	while (iter != params.end()) {
		rpl += *iter;
		++iter;
		if (iter != params.end()) {
			rpl += " ";
		}
	}
	rpl += "\n";
}

User *Server::findByNick(const std::string &nick) {
	std::map<int, User*>::const_iterator	it;

	it = _users.begin();
	while (it != _users.end()) {
		if (it->second->getNick() == nick) {
			return it->second;
		}
		++it;
	}
	return nullptr;
}

void Server::startServer() {
	User*	user;
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int inFd;

	if (!_err.empty()) {
		_err.clear();
	}

	_configuration = new Configuration();
	_configuration->parseConfiguration("../resources/configuration.yaml");
	_name = _configuration->getConfig().at("server.name");
	setupStruct();
	createSocket();
	bindSocketToPort();
	freeaddrinfo(_serverInfo);
	startListening();
	initPollFdSet();
	prepareMotd();

	std::cout << "Server waiting for connection" << std::endl;

	while (true) {
		int pollCount = poll(_pollFds, _fdPollSize, -1);

		if (pollCount == -1) {
			_err.append("An error on poll: ");
			_err.append(strerror(errno));
			throw RuntimeServerError(_err.c_str());
		}

		for (int i = 0; i < _fdPollSize; i++) {

			if (_pollFds[i].revents & POLLIN) {
				if (_pollFds[i].fd == _socketFd) {
					addrLen = sizeof remoteAddr;
					inFd = accept(_socketFd, reinterpret_cast<sockaddr*>(&remoteAddr), &addrLen);
					if (inFd == -1) {
						std::cerr << "Accept error: " << strerror(errno) << std::endl;
					} else {
						user = new User(inFd, addToPollSet(inFd), inet_ntoa(reinterpret_cast<sockaddr_in*>(&remoteAddr)->sin_addr));
						_users.insert(std::make_pair(inFd, user));
					}
				} else {
					receiveMessage(i);
					disconnectUsers();
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
