#include "../includes/Server.hpp"

int	gStopped = 0;

Server::Server(const char* port, const char* password) : _port(port), _pass(password) {
	_serverInfo = nullptr;
	_pollFds = nullptr;
	_configuration = nullptr;
	_socketFd = -1;
	_fdPollSize = 5;
	_registrationTimeOut = 60;
	_maxNbrOfConnections = 20;
}

Server::~Server() {
	if (!_users.empty()) {
		std::map<int, User*>::iterator	it;

		it = _users.begin();
		while (it != _users.end()) {
			close(it->second->getFd());
			delete it->second;
			++it;
		}
	}
	if (!_channels.empty()) {
		std::map<std::string, Channel*>::iterator	it;

		it = _channels.begin();
		while (it != _channels.end()) {
			delete it->second;
			++it;
		}
	}
	if (_pollFds) free(_pollFds);
	delete _configuration;
}

void Server::handleSignals(int signum) {
	if (signum == SIGSEGV) {
		std::cerr << "Runtime error" << std::endl;
		exit(EXIT_FAILURE);
	}
	gStopped = 1;
}

void Server::dieCmd(User *user, const std::string &cmd,
					const std::vector<std::string> &params) {
	(void)cmd;
	(void)params;
	if (!user->isOperator()) {
		sendError(user, ERR_NOPRIVILEGES);
	} else {
		gStopped = 1;
	}
}

void Server::acceptUser() {
	struct sockaddr_storage	remoteAddr = {};
	User*					user;
	socklen_t				addrLen;
	int						inFd;
	int						connectionNbr;

	addrLen = sizeof remoteAddr;
	inFd = accept(_socketFd, reinterpret_cast<sockaddr*>(&remoteAddr), &addrLen);
	if (inFd == -1) {
		std::cerr << "Accept error: " << strerror(errno) << std::endl;
	} else {
		connectionNbr = addToPollSet(inFd);
		if (connectionNbr != -1) {
			user = new User(inFd, connectionNbr, inet_ntoa(reinterpret_cast<sockaddr_in*>(&remoteAddr)->sin_addr));
			_users.insert(std::make_pair(inFd, user));
			_unregisteredUsers.insert(std::make_pair(inFd, user));
		} else {
			std::string rpl = ":" + _name
							  + " NOTICE * :The maximum number of connections has been reached\n";
			sendAll(rpl.c_str(), rpl.size(), inFd);
			close(inFd);
		}
	}
}

void Server::mainLoop() {
	int						userToCheck;
	int						timeOut;
	int						pollCount;

	while (!gStopped) {
		timeOut = findMinTimeOut(&userToCheck);
		pollCount = poll(_pollFds, _fdPollSize, timeOut);

		if (pollCount == -1) {
			_err.append("An error on poll: ");
			_err.append(strerror(errno));
			throw RuntimeServerError(_err.c_str());
		}

		for (int i = 0, j = 0; i < _fdPollSize && j < pollCount; i++) {

			if (_pollFds[i].revents & POLLIN) {
				if (_pollFds[i].fd == _socketFd) {
					acceptUser();
				} else {
					receiveMessage(i);
					disconnectUsers();
				}
				++j;
			}
		}
		if (userToCheck != -1) {
			checkRegistrationTimeOut(userToCheck);
		}
	}
}

void Server::startServer() {
	configureServer();
	setupStruct();
	createSocket();
	bindSocketToPort();
	freeaddrinfo(_serverInfo);
	startListening();
	initPollFdSet();
	prepareMotd();

	std::signal(SIGINT, handleSignals);
	std::signal(SIGSEGV, handleSignals);

	mainLoop();
	killServer();
	std::signal(SIGINT, SIG_DFL);
	std::signal(SIGSEGV, SIG_DFL);
}

Server::LaunchFailed::LaunchFailed(const char *msg) : std::runtime_error(msg) {}

Server::RuntimeServerError::RuntimeServerError(const char *msg) : std::runtime_error(msg) {}
