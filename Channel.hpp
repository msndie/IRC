#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include <string>
#include "User.hpp"

class Channel {
private:
	User				*_owner;
	std::string			_name;
	std::string			_topic;
	std::string			_pass;
	std::list<User*>	_users;

public:
	explicit Channel(User *owner, const std::string &name, const std::string &pass="");
	Channel(User *owner, const std::string &name, const std::string &topic, const std::string &pass="");
	~Channel();

	void					addUser(User *user);
	void					changeTopic(const std::string &topic);
	void					removerUser(const std::string &nick);
	void					changeOwner();
	User					*getOwner() const;
	const std::string		&getName() const;
	const std::string		&getTopic() const;
	const std::string		&getPass() const;
	const std::list<User*>	&getUsers() const;
};

#endif /*IRC_CHANNEL_HPP*/
