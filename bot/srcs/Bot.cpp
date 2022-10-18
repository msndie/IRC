#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sys/poll.h>
#include <csignal>
#include "../includes/Bot.hpp"
#include "../includes/utils.h"

int	gStopped = 0;

Bot::Bot(const char *config, const char *puns) : _config(nullptr), _socket(-1), _counter(0), _active(
		false) {
	_punsPath = puns;
	_configPath = config;
}

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

void Bot::loadPuns() {
	std::ifstream	ifs;
	std::string		line;

	ifs.open(_punsPath, std::ifstream::in);
	if (!ifs.is_open()) {
		throw LaunchFailed("Can't open Puns file");
	}
	while (std::getline(ifs, line)) {
		if (!line.empty() && !isStrBlank(line.c_str()))
			_puns.push_back(line);
	}
	if (_puns.empty()) {
		throw LaunchFailed("Puns content error");
	}
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
	if (connect(_socket, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
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
			if (_counter == _puns.size()) _counter = 0;
			rpl = "NOTICE " + nick + " :" + _puns[_counter++] + "\n";
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

void Bot::handleSignals(int signum) {
	if (signum == SIGSEGV) {
		std::cerr << "Runtime error" << std::endl;
		exit(EXIT_FAILURE);
	}
	gStopped = 1;
}

void Bot::startBot() {
	struct pollfd	pollFd = {};
	int				count;

	_config = new Configuration();
	_config->parseConfiguration(_configPath);
	loadPuns();
	createSocketAndConnect();
	pollFd.fd = _socket;
	pollFd.events = POLLIN;
	if (auth()) {
		throw LaunchFailed("Authentication failed");
	}
	_active = true;
	std::signal(SIGINT, handleSignals);
	std::signal(SIGSEGV, handleSignals);
	while (_active && !gStopped) {
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
