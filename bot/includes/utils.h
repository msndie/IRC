#ifndef UTILS_H
#define UTILS_H

#include <list>
#include <string>

bool						isStrBlank(const char *str);
bool						endsWith(const std::string& str, const std::string& suffix);
std::list< std::string >	split(const std::string& str, char delimiter);
int							sendAll(const char *msg, unsigned long nbrOfBytes, int receiverFd);

#endif /*UTILS_H*/
