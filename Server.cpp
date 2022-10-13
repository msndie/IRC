#include <fstream>
#include "Server.hpp"

int	gStopped = 0;

Server::Server(const char* port, const char* password) : _port(port), _pass(password) {
	_serverInfo = nullptr;
	_pollFds = nullptr;
	_configuration = nullptr;
	_socketFd = -1;
	_fdPollSize = 5;
}

Server::~Server() {
	if (_pollFds) free(_pollFds);
	delete _configuration;
}

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

void Server::handleSignals(int signum) {
	if (signum == SIGSEGV) {
		std::cerr << "Runtime error" << std::endl;
		exit(EXIT_FAILURE);
	}
	gStopped = 1;
}

void Server::configureServer() {
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
		_maxNbrOfConnections = 20;
	} else {
		std::cout << "Max number of connections is " << tmp << std::endl;
		_maxNbrOfConnections = tmp;
	}
	if (timeOut == _configuration->getConfig().end()
		|| (tmp = std::atoi(timeOut->second.c_str())) < 10 || tmp > 180) {
		std::cout << "Using default registration time out" << std::endl;
		_registrationTimeOut = 60;
	} else {
		std::cout << "Registration time out is " << tmp << std::endl;
		_registrationTimeOut = tmp;
	}
}

int Server::findMinTimeOut(int *connectionNbr) {
	std::time_t	now = std::time(nullptr);
	std::map<int, User*>::iterator	it;
	long	timeOut = _registrationTimeOut + 1;
	long	diff;

	it = _unregisteredUsers.begin();
	while (it != _unregisteredUsers.end()) {
		diff = now - it->second->getTimeOfConnection();
		if (_registrationTimeOut - diff < timeOut) {
			*connectionNbr = it->second->getConnectionNbr();
			timeOut = _registrationTimeOut - diff;
		}
		++it;
	}
	if (timeOut < 0) timeOut = 0;
	if (timeOut == _registrationTimeOut + 1) {
		*connectionNbr = -1;
		timeOut = -1;
	}
	return timeOut;
}

void Server::startServer() {
	User*	user;
	struct sockaddr_storage remoteAddr;
	socklen_t addrLen;
	int inFd;
	int connectionNbr;
	int userToCheck;

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

	std::cout << "Server waiting for connection" << std::endl;

	while (!gStopped) {
		int timeOut = findMinTimeOut(&userToCheck);
		int pollCount = poll(_pollFds, _fdPollSize, timeOut);

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
				} else {
					receiveMessage(i);
					disconnectUsers();
				}
			}
		}
		if (userToCheck != -1) {
			std::time_t	now = std::time(nullptr);
			std::map<int, User*>::iterator it;
			it = _unregisteredUsers.find(_pollFds[userToCheck].fd);
			if (it != _unregisteredUsers.end()
				&& (now - it->second->getTimeOfConnection()) >= _registrationTimeOut) {
					std::string rpl = ":" + _name + " NOTICE * :Registration time out\n";
					sendAll(rpl.c_str(), rpl.size(), it->second->getFd());
					deleteFromPollSet(it->second->getConnectionNbr());
					close(it->first);
					delete it->second;
					_unregisteredUsers.erase(it->first);
					_users.erase(it->first);
			}
		}
	}
	std::signal(SIGINT, SIG_DFL);
	std::signal(SIGSEGV, SIG_DFL);
}

const char *Server::LaunchFailed::what() const throw() {
	return msg;
}

const char *Server::RuntimeServerError::what() const throw() {
	return msg;
}
