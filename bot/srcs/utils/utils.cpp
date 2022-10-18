#include <list>
#include <string>

std::list< std::string > split(const std::string& str, char delimiter) {
	std::list< std::string >	list;
	std::string::const_iterator	it = str.begin();

	while (it != str.end()) {
		while (it != str.end() && *it == delimiter)
			++it;
		std::string::const_iterator itTwo = std::find(it, str.end(), delimiter);
		if (it != str.end()) {
			list.push_back(std::string(it, itTwo));
			it = itTwo;
		}
	}
	return list;
}

bool	isStrBlank(const char *str) {
	int i = 0;

	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '\0')
		return true;
	return false;
}

bool	endsWith(const std::string& str, const std::string& suffix) {
	if (str.length() >= suffix.length()) {
		return (0 == str.compare (str.length() - suffix.length(), suffix.length(), suffix));
	} else {
		return false;
	}
}
