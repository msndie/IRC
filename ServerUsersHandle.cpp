#include "Server.hpp"

int Server::addToPollSet(int inFd) {
	int i = 0;

	while (i < _fdPollSize) {
		if (_pollFds[i].fd == -1) break;
		++i;
	}
	if (i == _fdPollSize) {
		_fdPollSize *= 2;
		_pollFds = static_cast<pollfd *>(reallocf(_pollFds,
												  (sizeof(struct pollfd) *
												   _fdPollSize)));
		if (_pollFds == nullptr) {
			close(_socketFd);
			throw RuntimeServerError("Malloc error");
		}
		for (int j = i + 1; j < _fdPollSize; ++j) {
			_pollFds[j].fd = -1;
		}
	}
	_pollFds[i].fd = inFd;
	_pollFds[i].events = POLLIN;
	return i;
}

void Server::deleteFromPollSet(int i) {
	_pollFds[i].fd = -1;
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

bool	Server::isExistsByNick(const std::string &nick, int fd) {
	std::map<int, User*>::const_iterator	it;

	it = _users.begin();
	while (it != _users.end()) {
		if (it->second->getFd() != fd && it->second->isRegistered() && it->second->getNick() == nick) {
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
	}
	user->setDisconnect(true);
}

void Server::changeNick(User *user, const std::string& nick) {
	std::string	message;

	message += ":" + user->getInfo() + " NICK :" + nick + "\n";
	user->setNick(nick);
	sendAll(message.c_str(), message.size(), user->getFd());
}

void	Server::disconnectUsers() {
	std::map<int, User*>::iterator	it;

	it = _users.begin();
	while (it != _users.end()) {
		if (it->second->isDisconnect()) {
			deleteFromPollSet(it->second->getConnectionNbr());
			close(it->first);
			delete it->second;
			it = _users.erase(it);
		} else {
			++it;
		}
	}
}
