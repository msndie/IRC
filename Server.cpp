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

void Server::deleteChannel(Channel *channel) {
	_channels.erase(channel->getName());
	delete channel;
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
				concatMsgs(rpl, params, 1);
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
					concatMsgs(rpl, params, 1);
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

void Server::operCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.size() < 2) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		std::string name = params[0];
		std::string pass = params[1];
		std::map<std::string, std::string>::const_iterator it;

		it = _configuration->getConfig().find("server.operators." + name);
		if (it == _configuration->getConfig().end() || it->second != pass) {
			sendError(user, ERR_PASSWDMISMATCH);
			return;
		}
		std::string	rpl = ":" + _name + " " + std::to_string(RPL_YOUREOPER)
				+ " " + user->getNick() + " :You are now an IRC operator\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		rpl.clear();
		rpl += ":" + _name + " MODE " + user->getNick() + " +o\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		user->setOperator(true);
	}
}

void Server::pingCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.empty()) {
		sendError(user, ERR_NOORIGIN);
	} else {
		std::string rpl = "PONG " + _name + "\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
	}
}

void Server::killCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (!user->isOperator()) {
		sendError(user, ERR_NOPRIVILEGES);
	} else if (params.size() < 1) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		if (user->getNick() == params[0]) return;
		User *client = findByNick(params[0]);
		if (!client) {
			sendError(user, ERR_NOSUCHNICK, params[0]);
		} else {
			std::list<Channel*>::iterator	it;
			Channel	*channel;
			std::string	rpl = ":" + user->getInfo() + " KILL " + params[0];
			if (params.size() > 1) {
				rpl += " :";
				concatMsgs(rpl, params, 1);
			} else {
				rpl += "\n";
			}
			sendAll(rpl.c_str(), rpl.size(), client->getFd());
			rpl.clear();
			rpl += ":" + client->getInfo() + " QUIT :Has been killed by "
					+ user->getNick() + "\n";
			client->setDisconnect(true);
			it = client->getChannels().begin();
			while (it != client->getChannels().end()) {
				(*it)->removerUser(client, rpl, client->getFd());
				if ((*it)->isAlive()) {
					++it;
				} else {
					channel = *it;
					deleteChannel(channel);
					it = client->getChannels().erase(it);
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
