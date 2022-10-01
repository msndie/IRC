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
#include "User.hpp"

#define BACKLOG 5

class Server {
private:
	std::map< int, User* >	_users;
	struct addrinfo			*_serverInfo;
	const char				*_port;
	int 					_socketFd;
	std::string				_err;
	struct pollfd			*_pollFds;
	int						_fdCount;
	int						_fdPollSize;
	std::string				_pass;

	void	setupStruct();
	void	createSocket();
	void 	bindSocketToPort();
	void	startListening();
	void	addToPollSet(int inFd);
	void	deleteFromPollSet(int i);
	void	sendAll(const char *msg, ssize_t nbrOfBytes, int receiverFd);
	void	sendMessageToAll(const char *msg, ssize_t nbrOfBytes, int senderFd);
	void	receiveMessage(int connectionNbr);
	void	processMessages(User *user);
	void	sendMOTD(User *user);
	void	checkUserInfo(User *user);
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
