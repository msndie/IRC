#include "Channel.hpp"
#include "utils/utils.h"

Channel::~Channel() {}

Channel::Channel(User *owner, const std::string &name) : _owner(owner), _name(name) {
	_alive = true;
}

void Channel::addUser(User *user) {
	_users.push_back(user);
}

void Channel::changeTopic(const std::string &topic) {
	_topic = topic;
}

void Channel::removerUser(User *user, const std::string &msg) {
	std::list<User*>::iterator	it;

	if (_owner == user && !_users.empty()) {
		changeOwner();
		notifyAllUsers(msg);
	} else if (_owner == user && _users.empty()) {
		_alive = false;
	} else {
		it = _users.begin();
		while (it != _users.end()) {
			if ((*it) == user) {
				_users.erase(it);
				break;
			} else {
				++it;
			}
		}
		notifyAllUsers(msg);
	}
}

void Channel::changeOwner() {
	_owner = _users.front();
	_users.pop_front();
	notifyAllUsers(":IRC-server MODE " + _name + " +o " + _owner->getNick() + "\n");
}

User *Channel::getOwner() const {
	return _owner;
}

const std::string &Channel::getName() const {
	return _name;
}

const std::string &Channel::getTopic() const {
	return _topic;
}

const std::list<User *> &Channel::getUsers() const {
	return _users;
}

bool Channel::isAlive() const {
	return _alive;
}

void Channel::notifyAllUsers(const std::string &msg, int fd) {
	std::list<User*>::iterator	it;

	if (msg.empty()) {
		return;
	}
	if (_owner->getFd() != fd) {
		sendAll(msg.c_str(), msg.size(), _owner->getFd());
	}
	it = _users.begin();
	while (it != _users.end()) {
		if ((*it)->getFd() != fd) {
			sendAll(msg.c_str(), msg.size(), (*it)->getFd());
		}
		++it;
	}
}

bool Channel::isNameValid(const std::string &name) {
	std::string	specials = ", :";
	std::string::size_type len;

	len = name.length();
	if (len > 50) {
		return false;
	}
	for (std::string::size_type i = 0; i < len; ++i) {
		if ((i == 0 && name[i] != '#')
			|| specials.find(name[i]) != std::string::npos) {
			return false;
		}
	}
	return true;
}

void Channel::sendTopicInfo(User *user, bool toAll) {
	std::string	rpl;

	if (_topic.empty()) {
		rpl += ":IRC-server " + std::to_string(RPL_NOTOPIC) + " "
			   + user->getNick() + " " + _name + " :No topic is set\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
	} else {
		rpl += ":IRC-server " + std::to_string(RPL_TOPIC) + " "
			   + user->getNick() + " " + _name + " :"
			   + _topic + "\n";
		if (toAll) {
			notifyAllUsers(rpl);
		} else {
			sendAll(rpl.c_str(), rpl.size(), user->getFd());
		}
	}
}
