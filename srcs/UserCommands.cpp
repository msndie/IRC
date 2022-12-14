#include "../includes/Server.hpp"

void	Server::passCmd(User *user, const std::string &cmd,
						const std::vector<std::string> &params) {
	if (user->isRegistered()) {
		sendError(user, ERR_ALREADYREGISTRED);
	} else if (params.empty()) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		user->setPassword(params.front());
	}
}

void	Server::nickCmd(User *user, const std::string &cmd,
				const std::vector<std::string> &params) {
	if (!user->isRegistered() && user->getPassword().empty()) {
		std::string rpl = ":" + _name + " NOTICE * :Password required. Closing link\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		user->setDisconnect(true);
	} else if (params.empty()) {
		sendError(user, ERR_NONICKNAMEGIVEN, cmd);
	} else if (!isNicknameValid(params.front())) {
		sendError(user, ERR_ERRONEUSNICKNAME, params.front());
	} else if (isExistsByNick(params.front(), user->getFd())) {
		sendError(user, ERR_NICKNAMEINUSE, params.front());
	} else if (user->isRegistered() && user->getNick() != params.front()) {
		changeNick(user, params.front());
	} else {
		user->setNick(params.front());
	}
}

void	Server::userCmd(User *user, const std::string &cmd,
						const std::vector<std::string> &params) {
	if (!user->isRegistered() && user->getPassword().empty()) {
		std::string rpl = ":" + _name + " NOTICE * :Password required. Closing link\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		user->setDisconnect(true);
	} else if (user->isRegistered()) {
		sendError(user, ERR_ALREADYREGISTRED);
	} else if (params.size() < 4) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		user->setUsername(params[0]);
		user->setFullName(params[3]);
	}
}

void	Server::quitCmd(User *user, const std::string &cmd,
						const std::vector<std::string> &params) {
	(void)cmd;
	std::list<Channel*>::iterator	it;
	std::string	repl = ":" + user->getInfo() + " QUIT :";
	Channel *channel;
	std::set<int> fds;

	user->setDisconnect(true);
	if (params.empty()) {
		repl += "Client quit\n";
	} else {
		repl += params.front() + "\n";
	}
	sendAll(repl.c_str(), repl.size(), user->getFd());
	fds.insert(user->getFd());
	it = user->getChannels().begin();
	while (it != user->getChannels().end()) {
		(*it)->removerUser(user, repl, &fds);
		if ((*it)->isAlive()) {
			++it;
		} else {
			channel = *it;
			deleteChannel(channel);
			it = user->getChannels().erase(it);
		}
	}
}
