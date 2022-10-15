#include "../includes/Channel.hpp"
#include "../includes/utils.h"

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

void Channel::removerUser(User *user, const std::string &msg, std::set<int> *fds) {
	std::list<User*>::iterator	it;

	if (_owner == user && !_users.empty()) {
		changeOwner();
		notifyAllUsers(msg, fds);
	} else if (_owner == user && _users.empty()) {
		_alive = false;
		notifyAllUsers(msg, fds);
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
		notifyAllUsers(msg, fds);
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

const std::list<User *> &Channel::getUsers() const {
	return _users;
}

bool Channel::isAlive() const {
	return _alive;
}

void Channel::notifyAllUsers(const std::string &msg, std::set<int> *fds) const {
	std::list<User*>::const_iterator	it;

	if (msg.empty()) {
		return;
	}
	if (fds == nullptr || fds->count(_owner->getFd()) == 0) {
		sendAll(msg.c_str(), msg.size(), _owner->getFd());
		if (fds != nullptr) fds->insert(_owner->getFd());
	}
	it = _users.begin();
	while (it != _users.end()) {
		if (fds == nullptr || fds->count((*it)->getFd()) == 0) {
			sendAll(msg.c_str(), msg.size(), (*it)->getFd());
			if (fds != nullptr) fds->insert((*it)->getFd());
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

void Channel::sendTopicInfo(User *user, bool toAll) const {
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

void Channel::fillStatsForList(std::string &str) const {
	str += _name + " " + std::to_string(_users.size() + 1) + " :" + _topic + "\n";
}

void Channel::fillNicksForNames(std::string &str) const {
	std::list<User*>::const_iterator	it;

	str += _name + " :@" + _owner->getNick();
	if (_users.empty()) {
		str += "\n";
		return;
	}
	str += " ";
	it = _users.begin();
	while (it != _users.end()) {
		str += (*it)->getNick();
		++it;
		if (it != _users.end()) {
			str += " ";
		}
	}
	str += "\n";
}
