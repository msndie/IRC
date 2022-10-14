#include "Server.hpp"

void Server::deleteChannel(Channel *channel) {
	_channels.erase(channel->getName());
	delete channel;
}

void Server::concatMsgs(std::string &rpl, const std::vector<std::string> &params, int start) {
	std::vector<std::string>::const_iterator iter;

	iter = params.begin();
	while (start != 0) {
		++iter;
		--start;
	}
	while (iter != params.end()) {
		rpl += *iter;
		++iter;
		if (iter != params.end()) {
			rpl += " ";
		}
	}
	rpl += "\n";
}

User *Server::findByNick(const std::string &nick) {
	std::map<int, User*>::const_iterator	it;

	it = _users.begin();
	while (it != _users.end()) {
		if (it->second->getNick() == nick) {
			return it->second;
		}
		++it;
	}
	return nullptr;
}

int Server::findMinTimeOut(int *connectionNbr) {
	std::time_t	now = std::time(nullptr);
	std::map<int, User*>::iterator	it;
	long	timeOut = _registrationTimeOut + 1;
	long	diff;

	it = _unregisteredUsers.begin();
	while (it != _unregisteredUsers.end()) {
		diff = now - it->second->getTimeOfConnection();
		if (_registrationTimeOut - diff < timeOut) {
			*connectionNbr = it->second->getConnectionNbr();
			timeOut = _registrationTimeOut - diff;
		}
		++it;
	}
	if (timeOut < 0) timeOut = 0;
	if (timeOut == _registrationTimeOut + 1) {
		*connectionNbr = -1;
		timeOut = -1;
	}
	return (int)timeOut;
}

void Server::killServer() {
	std::map<std::string, Channel*>::iterator itChannels;
	std::map<int, User*>::iterator itUsers;

	itChannels = _channels.begin();
	while (itChannels != _channels.end()) {
		delete itChannels->second;
		itChannels = _channels.erase(itChannels);
	}
	itUsers = _users.begin();
	std::string nameAndCmd = ":" + _name + " NOTICE ";
	std::string rpl;
	while (itUsers != _users.end()) {
		rpl.clear();
		rpl += nameAndCmd + itUsers->second->getNick() + " :Server has been killed.\n";
		sendAll(rpl.c_str(), rpl.size(), itUsers->second->getFd());
		close(itUsers->second->getFd());
		delete itUsers->second;
		itUsers = _users.erase(itUsers);
	}
	close(_socketFd);
}

void Server::checkRegistrationTimeOut(int nbrOfConnection) {
	std::time_t	now = std::time(nullptr);
	std::map<int, User*>::iterator it;
	it = _unregisteredUsers.find(_pollFds[nbrOfConnection].fd);
	if (it != _unregisteredUsers.end()
		&& (now - it->second->getTimeOfConnection()) >= _registrationTimeOut) {
		std::string rpl = ":" + _name + " NOTICE * :Registration time out\n";
		sendAll(rpl.c_str(), rpl.size(), it->second->getFd());
		deleteFromPollSet(it->second->getConnectionNbr());
		close(it->first);
		delete it->second;
		_unregisteredUsers.erase(it->first);
		_users.erase(it->first);
	}
}
