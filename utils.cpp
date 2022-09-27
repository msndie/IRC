#include <cstring>
#include <climits>

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

bool isPortValid(const char* str) {
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

bool isStrBlank(const char *str) {
	int i = 0;

	while ((str[i] >= 9 && str[i] <= 13) || str[i] == 32)
		i++;
	if (str[i] == '\0')
		return true;
	return false;
}