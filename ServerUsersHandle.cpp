#include "Server.hpp"

void Server::addToPollSet(int inFd) {
	if (_fdCount == _fdPollSize) {
		_fdPollSize *= 2;
		_pollFds = static_cast<pollfd *>(reallocf(_pollFds,
												  (sizeof(struct pollfd) *
												   _fdPollSize)));
		if (_pollFds == nullptr) {
			close(_socketFd);
			throw RuntimeServerError("Malloc error");
		}
	}
	_pollFds[_fdCount].fd = inFd;
	_pollFds[_fdCount].events = POLLIN;
	_fdCount++;
}

void Server::deleteFromPollSet(int i) {
	_pollFds[i] = _pollFds[--_fdCount];
}

bool Server::isNicknameValid(const std::string &nick) {
	std::string	specials = "[]\\`_^{|}";
	std::string::size_type len;

	len = nick.length();
	if (len > 9) {
		return false;
	}
	for (std::string::size_type i = 0; i < len; ++i) {
		if (!isalnum(nick[i]) && specials.find(nick[i]) == std::string::npos) {
			if (i == len - 1 && nick[i] == '-') {
				break;
			}
			return false;
		}
	}
	return true;
}

bool	Server::isExistsByNick(const std::string &nick) {
	std::map<int, User*>::const_iterator	it;

	it = _users.begin();
	while (it != _users.end()) {
		if (it->second->isRegistered() && it->second->getNick() == nick) {
			return true;
		}
		++it;
	}
	return false;
}

void Server::checkUserInfo(User *user) {
	if (!user->getNick().empty() && !user->getUsername().empty()
		&& !user->getFullName().empty() && user->getPassword() == this->_pass) {
		user->setRegistered(true);
		sendGreeting(user);
		return;
	} else if (user->getPassword() != this->_pass) {
		sendError(user, ERR_PASSWDMISMATCH);
	} else if (isExistsByNick(user->getNick())) {
		sendError(user, ERR_NICKNAMEINUSE, user->getNick());
	}
	user->setDisconnect(true);
}
