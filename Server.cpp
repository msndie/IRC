#include "Server.hpp"

Server::Server(const char* port, const char* password) : _port(port), _pass(password) {
	_serverInfo = nullptr;
	_pollFds = nullptr;
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

void Server::startServer() {
	User*	user;
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int inFd;

	if (!_err.empty()) {
		_err.clear();
	}

	setupStruct();
	createSocket();
	bindSocketToPort();
	freeaddrinfo(_serverInfo);
	startListening();

	_pollFds = static_cast<pollfd *>(malloc(sizeof(struct pollfd) * _fdPollSize));
	if (_pollFds == nullptr) {
		close(_socketFd);
		throw LaunchFailed("Malloc error");
	}

	_pollFds[0].fd = _socketFd;
	_pollFds[0].events = POLLIN;
	_fdCount++;

	std::cout << "Server waiting for connection" << std::endl;

	while (true) {
		int pollCount = poll(_pollFds, _fdCount, -1);

		if (pollCount == -1) {
			_err.append("An error on poll: ");
			_err.append(strerror(errno));
			throw RuntimeServerError(_err.c_str());
		}

		for (int i = 0; i < _fdCount; i++) {

			if (_pollFds[i].revents & POLLIN) {
				if (_pollFds[i].fd == _socketFd) {
					addrLen = sizeof remoteAddr;
					inFd = accept(_socketFd, reinterpret_cast<sockaddr*>(&remoteAddr), &addrLen);
					if (inFd == -1) {
						std::cerr << "Accept error: " << strerror(errno) << std::endl;
					} else {
						addToPollSet(inFd);
						user = new User(inFd, _fdCount - 1, inet_ntoa(reinterpret_cast<sockaddr_in*>(&remoteAddr)->sin_addr));
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

void Server::deleteChannel(Channel *channel) {
	_channels.erase(channel->getName());
	delete channel;
}

static void concatMsgs(std::string &rpl, const std::vector<std::string> &params) {
	std::vector<std::string>::const_iterator iter;

	iter = ++params.begin();
	while (iter != params.end()) {
		rpl += *iter++ + " ";
	}
	rpl += "\n";
}

void Server::msgCmd(User *user, const std::string &cmd,
					const std::vector<std::string> &params, bool isNotice) {
	if (params.empty()) {
		if (!isNotice) sendError(user, ERR_NORECIPIENT);
	} else if (params.size() == 1) {
		if (!isNotice) sendError(user, ERR_NOTEXTTOSEND);
	} else {
		std::string	rpl;

		if (params[0][0] == '#') {
			try {
				Channel *channel = _channels.at(params[0]);
				if (!user->isOnChannel(channel)) {
					if (!isNotice) sendError(user, ERR_NOTONCHANNEL, params[0]);
					return;
				}
				rpl += ":" + user->getInfo() + (isNotice ? " NOTICE " : " PRIVMSG ") + params[0] + " :";
				concatMsgs(rpl, params);
				channel->notifyAllUsers(rpl, user->getFd());
			} catch (std::out_of_range &ex) {
				if (!isNotice) sendError(user, ERR_NOSUCHNICK, params[0]);
			}
		} else {
			std::map<int, User *>::iterator	it;

			it = _users.begin();
			while (it != _users.end()) {
				if (it->second->getNick() == params[0]) {
					rpl += ":" + user->getInfo() + (isNotice ? " NOTICE " : " PRIVMSG ")
							+ it->second->getNick() + " :";
					concatMsgs(rpl, params);
					sendAll(rpl.c_str(), rpl.size(), it->first);
					break;
				}
				++it;
			}
			if (it == _users.end()) {
				if (!isNotice) sendError(user, ERR_NOSUCHNICK, params[0]);
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
