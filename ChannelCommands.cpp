#include "Server.hpp"

static void sendChannelInfo(User *user, Channel *channel) {
	std::string	rpl;
	std::list<User *>::const_iterator it;

	if (channel->getTopic().empty()) {
		rpl += ":IRC-server " + std::to_string(RPL_NOTOPIC) + " "
				+ user->getNick() + " " + channel->getName() + " :No topic is set\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
	} else {
		rpl += ":IRC-server " + std::to_string(RPL_TOPIC) + " "
			   + user->getNick() + " " + channel->getName() + " :"
			   + channel->getTopic() + "\n";
		sendAll(rpl.c_str(), rpl.size(), user->getFd());
	}
	rpl.clear();
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
		Channel *channel = _channels[name];
		if (channel) {
			channel->addUser(user);
		} else {
			channel = new Channel(user, name);
			_channels.insert(std::make_pair(name, channel));
		}
		std::string ret = ":" + user->getInfo() + " JOIN " + name + "\n";
		sendAll(ret.c_str(), ret.size(), user->getFd());
		sendChannelInfo(user, channel);
	}
}
