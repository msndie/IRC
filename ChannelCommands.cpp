#include "Server.hpp"

static void sendChannelInfo(User *user, Channel *channel) {
	std::string	rpl;
	std::list<User *>::const_iterator it;

	channel->sendTopicInfo(user);
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
