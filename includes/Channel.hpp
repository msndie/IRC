#ifndef IRC_CHANNEL_HPP
#define IRC_CHANNEL_HPP

#include <string>
#include <set>
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
	void					removerUser(User *user, const std::string &msg, std::set<int> *fds = nullptr);
	void					changeOwner();
	User					*getOwner() const;
	const std::string		&getName() const;
	const std::list<User*>	&getUsers() const;
	bool					isAlive() const;
	void					notifyAllUsers(const std::string &msg, std::set<int> *fds = nullptr) const;
	void					sendTopicInfo(User *user, bool toAll) const;
	static bool				isNameValid(const std::string &name);
	void					fillStatsForList(std::string &str) const;
	void					fillNicksForNames(std::string &str) const;
};

#endif /*IRC_CHANNEL_HPP*/
