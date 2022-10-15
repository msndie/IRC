#include "../includes/Configuration.hpp"

Configuration::Configuration() {}

Configuration::~Configuration() {}

void Configuration::checkConfig() {
	if (_config.empty() || _config.find("server.name") == _config.end()
		|| _config.find("server.operators.sclam") == _config.end()) {
		throw ConfigurationFileError("File content error");
	}
}

void Configuration::parseConfiguration(const std::string &path) {
	std::vector<std::string>	totalVec;
	std::ifstream				ifs;
	std::string					totalLine;
	std::string					line;
	size_t						space_count;
	size_t						last_space_count = 0;

	ifs.open(path, std::ifstream::in);
	if (!ifs.is_open()) {
		throw ConfigurationFileError(("Failed to open file at " + path).c_str());
	}
	while (getline(ifs, line)) {
		if (line.front() == ' ') {
			space_count = 0;
			while (line.front() == ' ') {
				space_count++;
				line.erase(0, 2);
			}
			if (space_count < last_space_count) {
				while (space_count < last_space_count) {
					if (!totalVec.empty())
						totalVec.pop_back();
					last_space_count--;
				}
			}
			last_space_count = space_count;
		} else {
			totalVec.clear();
			last_space_count = 0;
		}
		if (line.back() == ':') {
			totalVec.push_back(line.substr(0, line.length() - 1) + ".");
		}
		else {
			totalLine.clear();
			for (std::vector<std::string>::iterator it = totalVec.begin(); it != totalVec.end(); ++it) {
				totalLine += *it;
			}
			_config.insert(make_pair(totalLine + line.substr(0, line.find(':')),
									 line.substr(line.find(':') + 2)));
		}
	}
	ifs.close();
	checkConfig();
}

const std::map<std::string, std::string> &Configuration::getConfig() const {
	return _config;
}

Configuration::ConfigurationFileError::ConfigurationFileError(const char *msg) : std::runtime_error(msg) {}
