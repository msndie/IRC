#ifndef UTILS_H
#define UTILS_H

int							parsePort(const char* str);
bool						isPortValid(const char* str);
bool						isStrBlank(const char *str);
bool						endsWith(const std::string& str, const std::string& suffix);
std::list< std::string >	split(const std::string& str, char delimiter);

#endif /*UTILS_H*/
