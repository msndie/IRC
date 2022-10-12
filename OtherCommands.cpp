#include "Server.hpp"

void Server::motdCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	std::string	rpl;

	if (_motd.empty()) {
		rpl += ":" + _name + " " + std::to_string(RPL_MOTD) + " " + user->getNick()
			   + " :Welcome to my server :D\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		rpl.clear();
	} else {
		std::string templ = ":" + _name + " " + std::to_string(RPL_MOTD)
							+ " " + user->getNick() + " :";
		std::list<std::string>::const_iterator	it;

		it = _motd.begin();
		while (it != _motd.end()) {
			rpl += templ + *it + "\n";
			sendAll(rpl.c_str(), rpl.size(), user->getFd());
			rpl.clear();
			++it;
		}
	}
}

void Server::pingCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.empty()) {
		sendError(user, ERR_NOORIGIN);
	} else {
		std::string rpl = "PONG " + _name + "\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
	}
}

void Server::msgCmd(User *user, const std::string &cmd,
					const std::vector<std::string> &params, bool isNotice) {
	if (params.empty()) {
		if (!isNotice) sendError(user, ERR_NORECIPIENT);
	} else if (params.size() == 1) {
		if (!isNotice) sendError(user, ERR_NOTEXTTOSEND);
	} else {
		std::string	rpl;

		if (params[0][0] == '#') {
			try {
				Channel *channel = _channels.at(params[0]);
				if (!user->isOnChannel(channel)) {
					if (!isNotice) sendError(user, ERR_NOTONCHANNEL, params[0]);
					return;
				}
				rpl += ":" + user->getInfo() + (isNotice ? " NOTICE " : " PRIVMSG ") + params[0] + " :";
				concatMsgs(rpl, params, 1);
				std::set<int> fds;
				fds.insert(user->getFd());
				channel->notifyAllUsers(rpl, &fds);
			} catch (std::out_of_range &ex) {
				if (!isNotice) sendError(user, ERR_NOSUCHNICK, params[0]);
			}
		} else {
			std::map<int, User *>::iterator	it;

			it = _users.begin();
			while (it != _users.end()) {
				if (it->second->getNick() == params[0]) {
					rpl += ":" + user->getInfo() + (isNotice ? " NOTICE " : " PRIVMSG ")
						   + it->second->getNick() + " :";
					concatMsgs(rpl, params, 1);
					sendAll(rpl.c_str(), rpl.size(), it->first);
					break;
				}
				++it;
			}
			if (it == _users.end()) {
				if (!isNotice) sendError(user, ERR_NOSUCHNICK, params[0]);
			}
		}
	}
}
