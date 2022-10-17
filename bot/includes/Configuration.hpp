#ifndef IRC_CONFIGURATION_HPP
#define IRC_CONFIGURATION_HPP

#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>

class Configuration {
private:
	std::map<std::string, std::string>	_config;

	void	checkConfig();

public:
	Configuration();
	~Configuration();

	void	parseConfiguration(const std::string &path);
	const std::map<std::string, std::string>	&getConfig() const;

	class ConfigurationFileError : public std::runtime_error {
		public:
			explicit ConfigurationFileError(const char* msg);
	};
};

#endif /*IRC_CONFIGURATION_HPP*/
