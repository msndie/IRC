#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include <string>
#include "User.hpp"

class User;

class Channel {
private:
	User				*_owner;
	std::string			_name;
	std::string			_topic;
	std::list<User*>	_users;
	bool				_alive;

public:
	explicit Channel(User *owner, const std::string &name);
	~Channel();

	void					addUser(User *user);
	void					changeTopic(const std::string &topic);
	void					removerUser(User *user, const std::string &msg);
	void					changeOwner();
	User					*getOwner() const;
	const std::string		&getName() const;
	const std::string		&getTopic() const;
	const std::list<User*>	&getUsers() const;
	bool					isAlive() const;
	void					notifyAllUsers(const std::string &msg, int fd = -1) const;
	void					sendTopicInfo(User *user, bool toAll) const;
	static bool				isNameValid(const std::string &name);
	void					fillStatsForList(std::string &str) const;
};

#endif /*IRC_CHANNEL_HPP*/
