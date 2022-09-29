#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <string>
#include "Message.hpp"

class User {
private:
	std::string	_nick;
	std::string	_fullName;
	std::string	_username;
	std::string	_host;
	char*		_remains;
	int			_fd;

public:
	User(int fd, const std::string& host);
	const std::string	&getHost() const;
	int					getFd() const;
	void				setRemains(const char* str);
	char*				getRemains() const;
	void 				freeRemains();
};


#endif /*IRC_USER_HPP*/
