#include "Channel.hpp"

Channel::~Channel() {}

Channel::Channel(User *owner, const std::string &name) : _owner(owner), _name(name) {}

void Channel::addUser(User *user) {
	_users.push_back(user);
}

void Channel::changeTopic(const std::string &topic) {
	_topic = topic;
}

void Channel::removerUser(const std::string &nick) {
	std::list<User*>::iterator	it;

	if (_owner->getNick() == nick) {
		changeOwner();
	}
	it = _users.begin();
	while (it != _users.end()) {
		if ((*it)->getNick() == nick) {
			it = _users.erase(it);
		} else {
			++it;
		}
	}
}

void Channel::changeOwner() {
	_owner = _users.front();
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
