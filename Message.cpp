#include <iostream>
#include "Message.hpp"
#include "utils/utils.h"

Message::Message(std::vector<std::string> params, std::string cmd,
				 std::string prefix) : _params(params), _cmd(cmd), _prefix(prefix) {}

Message::~Message() {}

void Message::parseMessages(const std::string& msg, std::list<Message*>& list) {
	std::list< std::string >	listMsgs;
	std::list< std::string >	listCmds;
	std::string					prefix;
	std::string					cmd;
	std::vector< std::string >	params;

	listMsgs = split(msg, '\n');
	while (!listMsgs.empty()) {

		listCmds = split(listMsgs.front(), ' ');

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
		list.push_back(new Message(params, cmd, prefix));
		cmd.clear();
		prefix.clear();
		params.clear();
	}
}

const std::string &Message::getCmd() const {
	return _cmd;
}

const std::string &Message::getPrefix() const {
	return _prefix;
}

const std::vector<std::string> &Message::getParams() const {
	return _params;
}

std::ostream &operator<<(std::ostream &os, const Message &message) {
	std::vector<std::string>::const_iterator it;

	it = message.getParams().begin();
	os << "Cmd: " << message.getCmd() << " Prefix: " << message.getPrefix() << " Params: ";
	while (it != message.getParams().end()) {
		os << *it;
		++it;
		if (it != message.getParams().end())
			os << " ";
	}
	return os;
}
