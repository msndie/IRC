#include "../includes/Server.hpp"

static void sendChannelInfo(User *user, Channel *channel) {
	std::string	rpl;
	std::list<User *>::const_iterator it;

	channel->sendTopicInfo(user, false);
	rpl += ":IRC-server " + std::to_string(RPL_NAMREPLY) + " " + user->getNick() + " = " + channel->getName() + " :";
	rpl += "@" + channel->getOwner()->getNick();
	it = channel->getUsers().begin();
	while (it != channel->getUsers().end()) {
		rpl += " " + (*it)->getNick();
		++it;
	}
	rpl += "\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
	rpl.clear();
	rpl += ":IRC-server " + std::to_string(RPL_ENDOFNAMES) + " "
			+ user->getNick() + " " + channel->getName() + " :End of /NAMES list\n";
	sendAll(rpl.c_str(), rpl.size(), user->getFd());
}

void	Server::joinCmd(User *user, const std::string &cmd, const std::vector<std::string> &params) {
	if (params.empty()) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		std::string	name = params[0];
		if (!Channel::isNameValid(name)) {
			sendError(user, ERR_NOSUCHCHANNEL, name);
			return;
		}
		Channel *channel;
		try {
			channel = _channels.at(name);
			if (user->isOnChannel(channel)) {
				return;
			}
			channel->addUser(user);
		} catch (std::out_of_range &ex) {
			channel = new Channel(user, name);
			_channels.insert(std::make_pair(name, channel));
		}
		user->addChannel(channel);
		std::string ret = ":" + user->getInfo() + " JOIN " + name + "\n";
		channel->notifyAllUsers(ret);
		sendChannelInfo(user, channel);
	}
}

void Server::topicCmd(User *user, const std::string &cmd,
					  const std::vector<std::string> &params) {
	Channel *channel = nullptr;
	bool	toAll = false;

	if (params.empty()) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
		return;
	}
	std::string name = params[0];
	try {
		channel = _channels.at(name);
	} catch (std::out_of_range &ex) {}
	if (channel == nullptr || !user->isOnChannel(channel)) {
		sendError(user, ERR_NOTONCHANNEL, name);
		return;
	}
	if (params.size() > 1) {
		if (channel->getOwner() != user) {
			sendError(user, ERR_CHANOPRIVSNEEDED, name);
			return;
		}
		channel->changeTopic(params[1]);
		toAll = true;
	}
	channel->sendTopicInfo(user, toAll);
}

void Server::partCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.empty()) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {

		try {
			Channel *channel = _channels.at(params[0]);
			if (!user->isOnChannel(channel)) {
				sendError(user, ERR_NOTONCHANNEL, params[0]);
			} else {
				std::string	rpl = ":" + user->getInfo() + " PART " + params[0] + "\n";
				channel->removerUser(user, rpl);
				user->removeChannel(channel->getName());
				if (!channel->isAlive()) {
					deleteChannel(channel);
				}
			}
		} catch (std::out_of_range &ex) {
			sendError(user, ERR_NOSUCHCHANNEL, params[0]);
		}

	}
}

void Server::kickCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	if (params.size() < 2) {
		sendError(user, ERR_NEEDMOREPARAMS, cmd);
	} else {
		try {
			Channel *channel = _channels.at(params[0]);
			if (!user->isOnChannel(channel)) {
				sendError(user, ERR_NOTONCHANNEL, params[0]);
			} else if (user != channel->getOwner()) {
				sendError(user, ERR_CHANOPRIVSNEEDED, params[0]);
			} else {
				User *client = findByNick(params[1]);
				if (!client || !client->isOnChannel(channel)) {
					sendError(user, ERR_USERNOTINCHANNEL, params[1] + " " + params[0]);
					return;
				}
				if (client == user) {
					return;
				}
				std::string	rpl = ":" + user->getInfo() + " KICK " + params[0] + " " + params[1] + " :";
				if (params.size() > 2) {
					concatMsgs(rpl, params, 2);
				} else {
					rpl += params[1] + "\n";
				}
				channel->notifyAllUsers(rpl);
				channel->removerUser(client, "");
				client->removeChannel(channel->getName());
			}
		} catch (std::out_of_range &ex) {
			sendError(user, ERR_NOSUCHCHANNEL, params[0]);
		}
	}
}

void Server::listCmd(User *user, const std::string &cmd,
					 const std::vector<std::string> &params) {
	(void)cmd;
	std::map<std::string, Channel*>::const_iterator	it;
	std::string templ = ":IRC-server " + std::to_string(RPL_LIST) + " "
			+ user->getNick() + " ";

	if (params.empty()) {
		it = _channels.begin();
		while (it != _channels.end()) {
			std::string str = templ;
			it->second->fillStatsForList(str);
			sendAll(str.c_str(), str.size(), user->getFd());
			++it;
		}
	} else {
		try {
			Channel *channel = _channels.at(params[0]);
			channel->fillStatsForList(templ);
			sendAll(templ.c_str(), templ.size(), user->getFd());
		} catch (std::out_of_range &ex) {
			sendError(user, ERR_NOSUCHNICK, params[0]);
		}
	}
	templ.clear();
	templ += ":IRC-server " + std::to_string(RPL_LISTEND) + " "
			+ user->getNick() + " :End of /LIST\n";
	sendAll(templ.c_str(), templ.size(), user->getFd());
}

void Server::namesCmd(User *user, const std::string &cmd,
					  const std::vector<std::string> &params) {
	(void)cmd;
	std::map<std::string, Channel*>::const_iterator	it;
	std::map<int, User*>::const_iterator itForUsers;
	std::string templ = ":IRC-server " + std::to_string(RPL_NAMREPLY) + " "
						+ user->getNick() + " = ";

	if (params.empty()) {
		it = _channels.begin();
		while (it != _channels.end()) {
			std::string str = templ;
			it->second->fillNicksForNames(str);
			sendAll(str.c_str(), str.size(), user->getFd());
			++it;
		}
		templ.clear();
		templ += ":IRC-server " + std::to_string(RPL_NAMREPLY) + " "
				+ user->getNick() + " * * :";
		itForUsers = _users.begin();
		while (itForUsers != _users.end()) {
			if (itForUsers->second->getChannels().empty()) {
				templ += itForUsers->second->getNick() + " ";
			}
			++itForUsers;
		}
		templ += "\n";
		sendAll(templ.c_str(), templ.size(), user->getFd());
		templ.clear();
		templ += ":IRC-server " + std::to_string(RPL_ENDOFNAMES) + " "
				+ user->getNick() + " * :End of /NAMES list.\n";
		sendAll(templ.c_str(), templ.size(), user->getFd());
	} else {
		try {
			_channels.at(params[0])->fillNicksForNames(templ);
			sendAll(templ.c_str(), templ.size(), user->getFd());
		} catch (std::out_of_range &ex) {}
		templ.clear();
		templ += ":IRC-server " + std::to_string(RPL_ENDOFNAMES) + " "
				 + user->getNick() + " " + params[0] + " :End of /NAMES list.\n";
		sendAll(templ.c_str(), templ.size(), user->getFd());
	}
}
