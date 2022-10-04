#include "Server.hpp"

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
	if (params.empty()) {
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
	if (user->isRegistered()) {
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
	std::string	repl = "QUIT :";
	user->setDisconnect(true);
	if (params.empty()) {
		repl += "Client quit\n";
	} else {
		repl += params.front() + "\n";
	}
	sendAll(repl.c_str(), repl.size(), user->getFd());
}
