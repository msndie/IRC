#ifndef IRC_BOT_HPP
#define IRC_BOT_HPP

#include "Configuration.hpp"
#include "Message.hpp"

class Bot {
private:
	std::list<Message*>			_messages;
	std::vector<std::string>	_puns;
	Configuration				*_config;
	std::string					_remains;
	std::string					_configPath;
	std::string					_punsPath;
	int							_socket;
	size_t 						_counter;
	bool						_active;

	void	createSocketAndConnect();
	void	readMessages();
	void	processMessages();
	int		auth();
	void	sendResponse(Message *message);
	void	deleteMessages();
	void	loadPuns();

	static void	handleSignals(int signum);

public:
	explicit Bot(const char *config, const char *puns);
	~Bot();
	void	startBot();

	class LaunchFailed : public std::runtime_error {
	public:
		explicit LaunchFailed(const char* msg);
	};
};

#endif /*IRC_BOT_HPP*/
