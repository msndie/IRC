#ifndef IRC_SERVER_HPP
#define IRC_SERVER_HPP

#include <iostream>
#include <exception>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>

#define BACKLOG 5

class Server {
private:
	struct addrinfo*	_serverInfo;
	const char*			_port;
	int 				_socketFd;
	std::string			_err;

	void	setupStructs();
	void	createSocket();

public:
	explicit Server(const char* port);
	~Server();

	void				startServer();

	class LaunchFailed : public std::exception {
		private:
			const char*	msg;
		public:
			explicit LaunchFailed(const char* msg) : msg(msg) {}
			virtual const char* what() const throw();
	};
};


#endif /*IRC_SERVER_HPP*/
