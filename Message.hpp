#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <map>
#include <string>
#include <cstring>
#include "Responses.hpp"

class Message {
private:
	std::map< std::string, std::string >	_values;
	Responses type;

	Message();
public:
	~Message();
	static Message	*parseMessage(const char* msg);
};


#endif //MESSAGE_HPP
