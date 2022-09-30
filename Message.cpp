#include <iostream>
#include "Message.hpp"
#include "utils/utils.h"

Message::Message(std::list<std::string> params, std::string cmd,
				 std::string prefix) : _params(params), _cmd(cmd), _prefix(prefix) {}

Message::~Message() {}

std::list< Message* > Message::parseMessages(std::string msg) {
	std::list< std::string >	listMsgs;
	std::list< std::string >	listCmds;
	std::list< Message* >		ret;
	std::string					msgTmp;
	std::string::size_type		pos;
	std::string					prefix;
	std::string					cmd;
	std::list< std::string >	params;

	while ((pos = msg.find("\r\n")) != std::string::npos)
		msg.replace(pos, 2, "\n");
	listMsgs = split(msg, '\n');
	while (!listMsgs.empty()) {

		msgTmp = listMsgs.front();
		listCmds = split(msgTmp, ' ');
		msgTmp.clear();

		if (!listCmds.empty() && listCmds.front()[0] == ':') {
			prefix = std::string(listCmds.front().begin() + 1, listCmds.front().end());
			listCmds.pop_front();
		}
		if (!listCmds.empty()) {
			cmd = listCmds.front();
			listCmds.pop_front();
		}
		while (!listCmds.empty()) {
			if (listCmds.front()[0] == ':') {
				std::string str(listCmds.front().begin() + 1, listCmds.front().end());
				listCmds.pop_front();
				while (!listCmds.empty()) {
					str += " ";
					str += listCmds.front();
					listCmds.pop_front();
				}
				params.push_back(str);
			} else {
				params.push_back(listCmds.front());
				listCmds.pop_front();
			}
		}
		listMsgs.pop_front();
		ret.push_back(new Message(params, cmd, prefix));
		cmd.clear();
		prefix.clear();
		params.clear();
	}
	return (ret);
}

const std::list<std::string> &Message::getParams() const {
	return _params;
}

const std::string &Message::getCmd() const {
	return _cmd;
}

const std::string &Message::getPrefix() const {
	return _prefix;
}

std::ostream &operator<<(std::ostream &os, const Message &message) {
	std::list< std::string >::const_iterator it = message.getParams().begin();
	os << "Cmd: " << message.getCmd() << " Prefix: " << message.getPrefix() << " Params: ";
	while (it != message.getParams().end()) {
		os << *it;
		++it;
		if (it != message.getParams().end())
			os << " ";
	}
	return os;
}
