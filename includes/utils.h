#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <string>

bool						isPortValid(const char* str);
bool						isStrBlank(const char *str);
bool						endsWith(const std::string& str, const std::string& suffix);
std::list< std::string >	split(const std::string& str, char delimiter);
void						sendAll(const char *msg, unsigned long nbrOfBytes, int receiverFd);
void						trim(std::string &s);

#endif /*UTILS_H*/
