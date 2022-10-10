#ifndef IRC_CONFIGURATION_HPP
#define IRC_CONFIGURATION_HPP

#include <map>
#include <string>

class Configuration {
private:
	std::map<std::string, std::string>	_config;

	void	checkConfig();

public:
	Configuration();
	~Configuration();

	void	parseConfiguration(const std::string &path);
	const std::map<std::string, std::string>	&getConfig() const;

	class ConfigurationFileError : public std::exception {
	private:
		const char*	msg;
	public:
		explicit ConfigurationFileError(const char* msg) : msg(msg) {}
		virtual const char* what() const throw();
	};
};

#endif /*IRC_CONFIGURATION_HPP*/
