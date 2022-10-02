#include "Server.hpp"

void	Server::receiveMessage(int connectionNbr) {
	ssize_t	bytes;
	char	buf[513] = { 0 };
	int		senderFd;
	User	*user;

	senderFd = _pollFds[connectionNbr].fd;
	user = _users[senderFd];
	bytes = recv(senderFd, buf, sizeof buf, 0);

	if (bytes <= 0) {

		if (bytes == 0) {
			std::cout << "Client with fd " << senderFd
					  << " close connection" << std::endl;
		} else {
			std::cerr << "Recv error: " << strerror(errno)
					  << std::endl;
		}
		close(senderFd);
		deleteFromPollSet(connectionNbr);
		_users.erase(senderFd);

	} else {

		user->setRemains(buf);
		if (endsWith(user->getRemains(), "\r\n") || endsWith(user->getRemains(), "\n")) {
			user->parseMessages();
			const std::list<Message *> &list = user->getMessages();
			std::list<Message *>::const_iterator it;
			it = list.begin();
			while (it != list.end()) {
				std::cout << *(*it);
				++it;
			}
			std::cout << std::endl;
			processMessages(user);
			user->clearMessages();
		}

	}
}

void Server::sendGreeting(User *user) {
	std::string servName = ":IRC-server ";
	std::string	rpl;

	rpl += servName + std::to_string(RPL_WELCOME) + " " + user->getNick()
		   + " :Welcome " + user->getNick() + "!" + user->getUsername()
		   + "@" + user->getHost() + "\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	std::cout << rpl << std::endl;
	rpl.clear();
	rpl += servName + std::to_string(RPL_MOTDSTART) + " " + user->getNick()
		   + " :Message of the day\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	std::cout << rpl << std::endl;
	rpl.clear();
	rpl += servName + std::to_string(RPL_MOTD) + " " + user->getNick()
		   + " :Welcome to my IRC-server :D\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	std::cout << rpl << std::endl;
	rpl.clear();
	rpl += servName + std::to_string(RPL_ENDOFMOTD) + " " + user->getNick()
		   + " :End of message of the day\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	std::cout << rpl << std::endl;
}

void Server::sendError(User *user, int errorCode, const std::string& arg) {
	std::string	rpl = ":IRC-server ";
	std::string	userName;

	if (user->getNick().empty()) {
		userName += user->getUsername();
	} else {
		userName += user->getNick();
	}
	rpl += std::to_string(errorCode) + " " + userName + " ";
	switch (errorCode) {
		case ERR_UNKNOWNCOMMAND:
			rpl += arg + " :Unknown command";
			break;
		case ERR_NEEDMOREPARAMS:
			rpl += arg + " :Not enough parameters";
			break;
		case ERR_ALREADYREGISTRED:
			rpl += ":You may not register";
			break;
		case ERR_NONICKNAMEGIVEN:
			rpl += ":No nickname given";
			break;
		case ERR_ERRONEUSNICKNAME:
			rpl += arg + " :Erroneus nickname";
			break;
		case ERR_NICKNAMEINUSE:
			rpl += arg + " :Nickname already in use";
			break;
		case ERR_NOTREGISTERED:
			rpl += ":You have not registered";
			break;
		case ERR_PASSWDMISMATCH:
			rpl += ":Password incorrect";
			break;
		default:
			rpl += ": Unknown error";
			break;
	}
	rpl += "\n";
	std::cout << rpl;
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
}

void Server::processMessages(User *user) {
	std::list<Message*>::const_iterator	it;

	it = user->getMessages().begin();
	while (it != user->getMessages().end()) {
		std::string cmd = (*it)->getCmd();
		if (cmd == "CAP") {
			++it;
			continue;
		}
		if (!user->isRegistered() && cmd != "QUIT" && cmd != "PASS"
			&& cmd != "USER" && cmd != "NICK") {
			sendError(user, ERR_NOTREGISTERED);
		} else {
			if (cmd == "PASS") {
				if (user->isRegistered()) {
					sendError(user, ERR_ALREADYREGISTRED);
				} else if ((*it)->getParams().empty()) {
					sendError(user, ERR_NEEDMOREPARAMS, cmd);
				} else {
					user->setPassword((*it)->getParams().front());
				}
			} else if (cmd == "NICK") {
				if ((*it)->getParams().empty()) {
					sendError(user, ERR_NEEDMOREPARAMS, cmd);
				} else if (!isNicknameValid((*it)->getParams().front())) {
					sendError(user, ERR_ERRONEUSNICKNAME, (*it)->getParams().front());
				} else if (isExistsByNick((*it)->getParams().front())) {
					sendError(user, ERR_NICKNAMEINUSE, (*it)->getParams().front());
				} else if (user->isRegistered()) {
//					TODO
				} else {
					user->setNick((*it)->getParams().front());
				}
			} else if (cmd == "USER") {
				if (user->isRegistered()) {
					sendError(user, ERR_ALREADYREGISTRED);
				} else if ((*it)->getParams().size() < 4) {
					sendError(user, ERR_NEEDMOREPARAMS, cmd);
				} else {
					user->setUsername((*it)->getParams()[0]);
					user->setFullName((*it)->getParams()[3]);
				}
			}
		}
		++it;
	}
	if (!user->isRegistered() && !user->getNick().empty()
		&& !user->getUsername().empty() && !user->getFullName().empty()
		&& !user->getPassword().empty()) {
		checkUserInfo(user);
	}
}
