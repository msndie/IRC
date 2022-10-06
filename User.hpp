#ifndef IRC_USER_HPP
#define IRC_USER_HPP

#include <string>
#include <list>
#include "Message.hpp"
#include "Channel.hpp"

class Channel;

class User {
private:
	std::list<Message*>	_messages;
	std::list<Channel*>	_channels;
	std::string			_nick;
	std::string			_fullName;
	std::string			_username;
	std::string			_password;
	std::string			_host;
	std::string			_remains;
	int					_fd;
	bool				_registered;
	bool				_disconnect;

public:
	User(int fd, const std::string& host);

	friend bool operator==(const User &lhs, const User &rhs);

	const std::string			&getHost() const;

	int							getFd() const;

	void						setRemains(const char* str);

	const std::string			&getRemains() const;

	void 						freeRemains();

	void						parseMessages();

	bool						isRegistered() const;

	void						setRegistered(bool registered);

	bool 						isDisconnect() const;

	void 						setDisconnect(bool disconnect);

	void						clearMessages();

	const std::list<Message *>	&getMessages() const;

	const std::string			&getNick() const;

	void						setNick(const std::string &nick);

	const std::string			&getFullName() const;

	void						setFullName(const std::string &fullName);

	const std::string			&getUsername() const;

	void						setUsername(const std::string &username);

	const std::string			&getPassword() const;

	void						setPassword(const std::string &password);

	std::string					getInfo() const;

	void						addChannel(Channel *channel);

	void						removeChannel(const std::string &name);

	std::list<Channel *>		&getChannels();
};

#endif /*IRC_USER_HPP*/
