#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include <iostream>
#include <exception>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <cstdlib>
#include <map>
#include <set>
#include "utils/utils.h"
#include <arpa/inet.h>
#include "User.hpp"
#include "Channel.hpp"
#include "Configuration.hpp"

#define BACKLOG 5

class Server {
private:
	std::map< std::string, Channel* >	_channels;
	std::map< int, User* >				_users;
	struct addrinfo						*_serverInfo;
	const char							*_port;
	int 								_socketFd;
	std::string							_err;
	struct pollfd						*_pollFds;
	int									_fdCount;
	int									_fdPollSize;
	std::string							_pass;
	std::string							_name;
	Configuration						*_configuration;

	void	setupStruct();
	void	createSocket();
	void 	bindSocketToPort();
	void	startListening();
	int		addToPollSet(int inFd);
	void	deleteFromPollSet(int i);
	void	initPollFdSet();

	void	receiveMessage(int connectionNbr);
	void	processMessages(User *user);
	void	sendGreeting(User *user);
	void	checkUserInfo(User *user);
	void	sendError(User *user, int errorCode, const std::string& arg="");
	void	disconnectUsers();
	void	deleteChannel(Channel *channel);

	bool	isExistsByNick(const std::string &nick, int fd=-1);
	bool	isNicknameValid(const std::string &nick);
	void	changeNick(User *user, const std::string& nick);
	User	*findByNick(const std::string &nick);
	void	concatMsgs(std::string &rpl, const std::vector<std::string> &params, int start);

	void	passCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	nickCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	userCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	quitCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	joinCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	msgCmd(User *user, const std::string &cmd, const std::vector<std::string> &params, bool isNotice);
	void	partCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	topicCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	kickCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	listCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	namesCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	operCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	pingCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void	killCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
public:
	explicit Server(const char* port, const char* password);
	~Server();

	void	startServer();

	class LaunchFailed : public std::exception {
		private:
			const char*	msg;
		public:
			explicit LaunchFailed(const char* msg) : msg(msg) {}
			virtual const char* what() const throw();
	};

	class RuntimeServerError : public std::exception {
	private:
		const char*	msg;
	public:
		explicit RuntimeServerError(const char* msg) : msg(msg) {}
		virtual const char* what() const throw();
	};
};

#endif /*IRC_SERVER_HPP*/
