#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <string>
#include <list>
#include "Message.hpp"

class User {
private:
	std::string	_nick;
	std::string	_fullName;
	std::string	_username;
	std::string	_host;
	std::string	_remains;
	int			_fd;
	Message		*_response;
	bool		_registered;

public:
	User(int fd, const std::string& host);
	const std::string	&getHost() const;
	int					getFd() const;
	void				setRemains(const char* str);
	const std::string	&getRemains() const;
	void 				freeRemains();
	Message				*getResponse();
	bool				isRegistered() const;
	void				setRegistered(bool registered);
};


#endif /*IRC_USER_HPP*/
