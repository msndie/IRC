#include "../includes/Server.hpp"

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
			quitCmd(user, "QUIT", std::vector<std::string>());
		} else {
			std::cerr << "Recv error: " << strerror(errno)
					  << std::endl;
		}

	} else {

		user->setRemains(buf);
		if (endsWith(user->getRemains(), "\r\n") || endsWith(user->getRemains(), "\n")) {
			user->parseMessages();
			processMessages(user);
			user->clearMessages();

		}

	}
}

void Server::sendGreeting(User *user) {
	std::string servName = ":" + _name + " ";
	std::string	rpl;

	rpl += servName + "00" + std::to_string(RPL_WELCOME) + " " + user->getNick()
		   + " :Welcome " + user->getInfo() + "\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	rpl.clear();
	rpl += servName + std::to_string(RPL_MOTDSTART) + " " + user->getNick()
		   + " :Message of the day\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	rpl.clear();

	motdCmd(user, "MOTD", std::vector<std::string>());

	rpl += servName + std::to_string(RPL_ENDOFMOTD) + " " + user->getNick()
		   + " :End of message of the day\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	rpl.clear();
}

void Server::sendError(User *user, int errorCode, const std::string& arg) {
	std::string	rpl = ":" + _name + " ";
	std::string	userName;

	if (user->getNick().empty() && user->getUsername().empty()) {
		userName += "*";
	} else if (user->getNick().empty()) {
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
		case ERR_BADCHANNELKEY:
			rpl += arg + " :Cannot join channel";
			break;
		case ERR_NOSUCHCHANNEL:
			rpl += arg + " :No such channel";
			break;
		case ERR_NORECIPIENT:
			rpl += ":No recipient given";
			break;
		case ERR_NOTEXTTOSEND:
			rpl += ":No text to send";
			break;
		case ERR_NOSUCHNICK:
			rpl += arg + " :No such nick/channel";
			break;
		case ERR_NOTONCHANNEL:
			rpl += arg + " :You're not on that channel";
			break;
		case ERR_CHANOPRIVSNEEDED:
			rpl += arg + " :You're not channel operator";
			break;
		case ERR_USERNOTINCHANNEL:
			rpl += arg + " :They aren't on that channel";
			break;
		case ERR_NOORIGIN:
			rpl += ":No origin specified";
			break;
		case ERR_NOPRIVILEGES:
			rpl += ":Permission Denied- You're not an IRC operator";
			break;
		default:
			rpl += ": Unknown error";
			break;
	}
	rpl += "\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
}

void Server::processMessages(User *user) {
	std::list<Message*>::const_iterator	it;

	it = user->getMessages().begin();
	while (it != user->getMessages().end()) {
		const std::string &cmd = (*it)->getCmd();
		if (cmd == "CAP") {
			++it;
			continue;
		}
		if (!user->isRegistered() && cmd != "QUIT" && cmd != "PASS"
			&& cmd != "USER" && cmd != "NICK") {
			sendError(user, ERR_NOTREGISTERED);
		} else {
			if (cmd == "PASS") {
				passCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "NICK") {
				nickCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "USER") {
				userCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "QUIT") {
				quitCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "JOIN") {
				joinCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "PRIVMSG") {
				msgCmd(user, cmd, (*it)->getParams(), false);
			} else if (cmd == "NOTICE") {
				msgCmd(user, cmd, (*it)->getParams(), true);
			} else if (cmd == "PART") {
				partCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "TOPIC") {
				topicCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "KICK") {
				kickCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "LIST") {
				listCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "NAMES") {
				namesCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "OPER") {
				operCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "PING") {
				pingCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "KILL") {
				killCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "MOTD") {
				motdCmd(user, cmd, (*it)->getParams());
			} else if (cmd == "DIE") {
				dieCmd(user, cmd, (*it)->getParams());
			} else {
				sendError(user, ERR_UNKNOWNCOMMAND, cmd);
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
