#include "Server.hpp"

void Server::operCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.size() < 2) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		std::string name = params[0];
		std::string pass = params[1];
		std::map<std::string, std::string>::const_iterator it;

		it = _configuration->getConfig().find("server.operators." + name);
		if (it == _configuration->getConfig().end() || it->second != pass) {
			sendError(user, ERR_PASSWDMISMATCH);
			return;
		}
		std::string	rpl = ":" + _name + " " + std::to_string(RPL_YOUREOPER)
							 + " " + user->getNick() + " :You are now an IRC operator\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		rpl.clear();
		rpl += ":" + _name + " MODE " + user->getNick() + " +o\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
		user->setOperator(true);
	}
}

void Server::killCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (!user->isOperator()) {
		sendError(user, ERR_NOPRIVILEGES);
	} else if (params.empty()) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		if (user->getNick() == params[0]) return;
		User *client = findByNick(params[0]);
		if (!client) {
			sendError(user, ERR_NOSUCHNICK, params[0]);
		} else {
			std::list<Channel*>::iterator	it;
			Channel	*channel;
			std::string	rpl = ":" + user->getInfo() + " KILL " + params[0];
			std::set<int> fds;

			if (params.size() > 1) {
				rpl += " :";
				concatMsgs(rpl, params, 1);
			} else {
				rpl += "\n";
			}
			sendAll(rpl.c_str(), rpl.size(), client->getFd());
			fds.insert(client->getFd());
			rpl.clear();
			rpl += ":" + client->getInfo() + " QUIT :Has been killed by "
				   + user->getNick() + "\n";
			client->setDisconnect(true);
			it = client->getChannels().begin();
			while (it != client->getChannels().end()) {
				(*it)->removerUser(client, rpl, &fds);
				if ((*it)->isAlive()) {
					++it;
				} else {
					channel = *it;
					deleteChannel(channel);
					it = client->getChannels().erase(it);
				}
			}
		}
	}
}
