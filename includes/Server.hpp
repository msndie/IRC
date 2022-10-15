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
#include <arpa/inet.h>
#include "csignal"
#include "utils.h"
#include "User.hpp"
#include "Channel.hpp"
#include "Configuration.hpp"

#define BACKLOG 5

class Server {
private:
	std::map< std::string, Channel* >	_channels;
	std::map< int, User* >				_users;
	std::map< int, User* >				_unregisteredUsers;
	struct addrinfo						*_serverInfo;
	const char							*_port;
	int 								_socketFd;
	std::string							_err;
	struct pollfd						*_pollFds;
	int									_fdPollSize;
	std::string							_pass;
	std::string							_name;
	Configuration						*_configuration;
	std::list<std::string>				_motd;
	int									_registrationTimeOut;
	int									_maxNbrOfConnections;

	void		setupStruct();
	void		createSocket();
	void 		bindSocketToPort();
	void		startListening();
	int			addToPollSet(int inFd);
	void		deleteFromPollSet(int i);
	void		initPollFdSet();
	void		prepareMotd();
	void		configureServer();
	void		killServer();
	void		mainLoop();

	static void	handleSignals(int signum);

	void		receiveMessage(int connectionNbr);
	void		processMessages(User *user);
	void		sendGreeting(User *user);
	void		checkUserInfo(User *user);
	void		sendError(User *user, int errorCode, const std::string& arg="");

	void		checkRegistrationTimeOut(int nbrOfConnection);
	void		disconnectUsers();
	void		deleteChannel(Channel *channel);
	int			findMinTimeOut(int *connectionNbr);
	bool		isExistsByNick(const std::string &nick, int fd=-1);
	bool		isNicknameValid(const std::string &nick);
	void		changeNick(User *user, const std::string& nick);
	User		*findByNick(const std::string &nick);
	void		concatMsgs(std::string &rpl, const std::vector<std::string> &params, int start);

	void		passCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		nickCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		userCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		quitCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		joinCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		msgCmd(User *user, const std::string &cmd, const std::vector<std::string> &params, bool isNotice);
	void		partCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		topicCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		kickCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		listCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		namesCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		operCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		pingCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		killCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		motdCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
	void		dieCmd(User *user, const std::string &cmd, const std::vector<std::string> &params);
public:
	explicit Server(const char* port, const char* password);
	~Server();

	void	startServer();

	class LaunchFailed : public std::runtime_error {
		public:
			explicit LaunchFailed(const char* msg);
	};

	class RuntimeServerError : public std::runtime_error {
		public:
			explicit RuntimeServerError(const char* msg);
	};
};

#endif /*IRC_SERVER_HPP*/
