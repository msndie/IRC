#include "User.hpp"
#include "utils/utils.h"

User::User(int fd, const std::string &host) : _fd(fd), _host(host), _response(nullptr) {
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

Message *User::getResponse() {

	delete _response;


	return NULL;
}

bool User::isRegistered() const {
	return _registered;
}

void User::setRegistered(bool registered) {
	_registered = registered;
}
