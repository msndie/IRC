#include <cstring>
#include <climits>
#include <cstdlib>
#include <cstddef>

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

bool endsWith(const char *str, const char *suffix) {
	if (!str || !suffix)
		return false;
	size_t strLen = strlen(str);
	size_t suffixLen = strlen(suffix);
	if (suffixLen > strLen)
		return false;
	return strncmp(str + strLen - suffixLen, suffix, suffixLen) == 0;
}

char	*strjoin(char const *s1, char const *s2) {
	char	*dest;
	int		i;
	int		j;
	int		n;

	i = 0;
	j = 0;
	n = 0;
	if (!s1 && !s2)
		return (strdup(""));
	if (!s1)
		return (strdup(s2));
	if (s2[0] == '\0')
		return (strdup(s1));
	dest = static_cast<char *>(malloc(
			sizeof(char) * (strlen(s1) + strlen(s2) + 1)));
	if (dest == nullptr)
		return (nullptr);
	while (s2[n] != '\0') {
		while (s1[j] != '\0')
			dest[i++] = s1[j++];
		dest[i++] = s2[n++];
	}
	dest[i] = '\0';
	return (dest);
}