#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sys/poll.h>
#include "../includes/Bot.hpp"
#include "../includes/utils.h"

Bot::Bot(const char *path) : _config(nullptr), _socket(-1), _active(false), _path(path) {}

Bot::~Bot() {
	if (!_messages.empty()) {
		std::list<Message*>::const_iterator	it;

		it = _messages.begin();
		while (it != _messages.end()) {
			delete *it;
			++it;
		}
	}
	delete _config;
	close(_socket);
}

void Bot::createSocketAndConnect() {
	int status;
	struct addrinfo hints = {};
	struct addrinfo *servinfo;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	status = getaddrinfo(_config->getConfig().find("server.ip")->second.c_str(),
						 _config->getConfig().find("server.port")->second.c_str(),
						 &hints, &servinfo);
	if (status != 0) {
		std::string err;
		err.append("getaddrinfo error: ");
		err.append(gai_strerror(status));
		throw LaunchFailed(err.c_str());
	}

	if ((_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		freeaddrinfo(servinfo);
		throw LaunchFailed("Socket creation failed");
	}
	if (connect(_socket, reinterpret_cast<const sockaddr *>(servinfo), sizeof(*servinfo)) == -1) {
		freeaddrinfo(servinfo);
		close(_socket);
		throw LaunchFailed("Failed to connect");
	}
	freeaddrinfo(servinfo);
}

int Bot::auth() {
	std::string pass = "PASS " + _config->getConfig().find("server.pass")->second + "\n";
	std::string nick = "NICK " + _config->getConfig().find("bot.nick")->second + "\n";
	std::string user = "USER " + _config->getConfig().find("bot.username")->second
					   + " * * :" + _config->getConfig().find("bot.realname")->second + "\n";

	if (sendAll(pass.c_str(), pass.size(), _socket)
		|| sendAll(nick.c_str(), nick.size(), _socket)
		|| sendAll(user.c_str(), user.size(), _socket)) {
		return (-1);
	}
	return (0);
}

void Bot::readMessages() {
	ssize_t					bytes;
	char					buf[513] = { 0 };
	std::string::size_type	pos;

	bytes = recv(_socket, buf, sizeof buf, 0);

	if (bytes == 0) {
		std::cout << "Server close connection" << std::endl;
		_active = false;
	} else if (bytes < 0) {
		std::cerr << "Recv error: " << strerror(errno) << std::endl;
	} else {
		_remains += buf;
		if (endsWith(_remains, "\r\n") || endsWith(_remains, "\n")) {
			while ((pos = _remains.find("\r\n")) != std::string::npos) {
				_remains.replace(pos, 2, "\n");
			}
			Message::parseMessages(_remains, _messages);
			_remains.clear();
		}
	}
}

void Bot::processMessages() {
	std::list<Message*>::const_iterator	it;

	it = _messages.begin();
	while (it != _messages.end()) {
		const std::string &cmd = (*it)->getCmd();

		if (cmd == "464" || cmd == "432" || cmd == "433") {
			throw LaunchFailed("Authentication failed");
		}
		if (cmd == "NOTICE" || cmd == "PRIVMSG") {
			sendResponse(*it);
		}
		++it;
	}
}

void Bot::sendResponse(Message *message) {
	std::string nick;
	std::string rpl;

	if (!message->getPrefix().empty()) {
		nick = message->getPrefix().substr(0, message->getPrefix().find('!'));
	}
	if (!nick.empty()) {
		std::string cmd = message->getParams()[1];
		if (cmd == "#make_a_joke") {
			rpl = "NOTICE " + nick + " :Why did Adele cross the road? To say hello from the other side.\n";
		} else {
			rpl = "NOTICE " + nick + " :available commands - #make_a_joke\n";
		}
		sendAll(rpl.c_str(), rpl.size(), _socket);
	}
}

void Bot::deleteMessages() {
	std::list<Message*>::const_iterator	it;

	it = _messages.begin();
	while (it != _messages.end()) {
		delete *it;
		++it;
	}
}

void Bot::startBot() {
	struct pollfd	pollFd = {};
	int				count;

	_config = new Configuration();
	_config->parseConfiguration(_path);
	createSocketAndConnect();
	pollFd.fd = _socket;
	pollFd.events = POLLIN;
	if (auth()) {
		throw LaunchFailed("Authentication failed");
	}
	_active = true;
	while (_active) {
		count = poll(&pollFd, 1, -1);
		if (count == -1) {
			std::string err;
			err.append("An error on poll: ");
			err.append(strerror(errno));
			throw std::runtime_error(err.c_str());
		}
		readMessages();
		if (!_messages.empty() && _active) {
			processMessages();
			deleteMessages();
			_messages.clear();
		}
	}
	close(_socket);
}

Bot::LaunchFailed::LaunchFailed(const char *msg) : runtime_error(msg) {}
