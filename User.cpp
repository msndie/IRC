#include <iostream>
#include "User.hpp"
#include "utils/utils.h"

User::User(int fd, const std::string &host) : _fd(fd), _host(host) {
	_disconnect = false;
	_registered = false;
}

const std::string &User::getHost() const {
	return _host;
}

int User::getFd() const {
	return _fd;
}

void User::setRemains(const char *str) {
	_remains += str;
}

const std::string &User::getRemains() const {
	return _remains;
}

void User::freeRemains() {
	_remains.clear();
}

bool User::isRegistered() const {
	return _registered;
}

void User::setRegistered(bool registered) {
	_registered = registered;
}

void User::parseMessages() {
	std::list<Message*>::const_iterator	it;
	std::string::size_type				pos;

	while ((pos = _remains.find("\r\n")) != std::string::npos)
		_remains.replace(pos, 2, "\n");
	Message::parseMessages(_remains, _messages);
	_remains.clear();
}

const std::list<Message *> &User::getMessages() const {
	return _messages;
}

void User::clearMessages() {
	_messages.clear();
}

bool User::isDisconnect() const {
	return _disconnect;
}

void User::setDisconnect(bool disconnect) {
	_disconnect = disconnect;
}

const std::string &User::getNick() const {
	return _nick;
}

void User::setNick(const std::string &nick) {
	_nick = nick;
}

const std::string &User::getFullName() const {
	return _fullName;
}

void User::setFullName(const std::string &fullName) {
	_fullName = fullName;
}

const std::string &User::getUsername() const {
	return _username;
}

void User::setUsername(const std::string &username) {
	_username = username;
}

const std::string &User::getPassword() const {
	return _password;
}

void User::setPassword(const std::string &password) {
	_password = password;
}