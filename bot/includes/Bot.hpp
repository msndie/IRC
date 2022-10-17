#ifndef IRC_BOT_HPP
#define IRC_BOT_HPP

#include "Configuration.hpp"
#include "Message.hpp"

class Bot {
private:
	Configuration		*_config;
	int					_socket;
	std::string			_remains;
	std::list<Message*>	_messages;
	bool				_active;
	std::string			_path;

	void	createSocketAndConnect();
	void	readMessages();
	void	processMessages();
	int		auth();
	void	sendResponse(Message *message);
	void	deleteMessages();

public:
	explicit Bot(const char *path);
	~Bot();
	void	startBot();

	class LaunchFailed : public std::runtime_error {
	public:
		explicit LaunchFailed(const char* msg);
	};
};

#endif /*IRC_BOT_HPP*/
