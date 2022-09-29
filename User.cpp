#include "User.hpp"

char	*strjoin(char const *s1, char const *s2);

User::User(int fd, const std::string &host) : _fd(fd), _host(host), _remains(nullptr) {
}

const std::string &User::getHost() const {
	return _host;
}

int User::getFd() const {
	return _fd;
}

void User::setRemains(const char *str) {
	char	*tmp;

	if (_remains) {
		tmp = _remains;
		_remains = strjoin(_remains, str);
		free(tmp);
	} else {
		_remains = strdup(str);
	}
}

char *User::getRemains() const {
	return _remains;
}

void User::freeRemains() {
	if (_remains) {
		free(_remains);
	}
	_remains = nullptr;
}
