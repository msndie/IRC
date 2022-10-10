#include <cstring>
#include <climits>
#include <list>
#include <string>

static void ltrim(std::string &s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(),
									std::not1(std::ptr_fun<int, int>(std::isspace))));
}

static void rtrim(std::string &s) {
	s.erase(std::find_if(s.rbegin(), s.rend(),
						 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
}

void trim(std::string &s) {
	ltrim(s);
	rtrim(s);
}

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

int	parsePort(const char* str) {
	int	result;
	int	i;

	result = 0;
	i = 0;
	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '-' || str[i] == '+' || strlen(str + i) > 5 || strlen(str + i) == 0)
		return (-1);
	while (str[i] <= '9' && str[i] >= '0') {
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (str[i] != '\0' || result > USHRT_MAX)
		return (-1);
	return (result);
}

bool	isPortValid(const char* str) {
	int	result;
	int	i;

	result = 0;
	i = 0;
//	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
//		i++;
	if (str[i] == '-' || str[i] == '+' || strlen(str) > 5 || strlen(str) == 0)
		return (false);
	while (str[i] <= '9' && str[i] >= '0') {
		result = result * 10 + (str[i] - '0');
		i++;
	}
	if (str[i] != '\0' || result > USHRT_MAX)
		return (false);
	return (true);
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
