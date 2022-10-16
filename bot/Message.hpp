#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <list>
#include <vector>
#include <string>
#include <cstring>
#include <ostream>

class Message {
private:
	std::vector< std::string >	_params;
	std::string					_cmd;
	std::string					_prefix;

	Message(std::vector<std::string> params, std::string cmd,
			std::string prefix);
public:
	~Message();
	static void						parseMessages(const std::string& msg, std::list<Message*>& list);
	const std::string				&getCmd() const;
	const std::string				&getPrefix() const;
	const std::vector<std::string>	&getParams() const;
};

std::ostream &operator<<(std::ostream &os, const Message &message);

#endif //MESSAGE_HPP
