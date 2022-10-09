#include "Server.hpp"

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
				sendAll(rpl.c_str(), rpl.size(), user->getFd());
				channel->removerUser(user, rpl);
				user->removeChannel(channel->getName());
				if (!channel->isAlive()) {
					deleteChannel(channel);
				}
			}
		} catch (std::out_of_range &ex) {
			sendError(user, ERR_NOSUCHNICK, params[0]);
		}

	}
}
