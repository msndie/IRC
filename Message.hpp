#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <list>
#include <string>
#include <cstring>
#include <ostream>
#include "Responses.hpp"

class Message {
private:
	std::list< std::string >	_params;
	std::string					_cmd;
	std::string					_prefix;

	Message(std::list<std::string> params, std::string cmd,
			std::string prefix);
public:
	~Message();
	static std::list< Message* >	parseMessages(std::string msg);
	const std::list<std::string>	&getParams() const;
	const std::string				&getCmd() const;
	const std::string				&getPrefix() const;
};

std::ostream &operator<<(std::ostream &os, const Message &message);

#endif //MESSAGE_HPP
