#ifndef IRC_BOT_HPP
#define IRC_BOT_HPP

#include "Configuration.hpp"

class Bot {
private:
	Configuration	*_config;

public:
	Bot();
	~Bot();
	void startBot();
};

#endif /*IRC_BOT_HPP*/
