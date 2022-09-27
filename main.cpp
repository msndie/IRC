#include <iostream>
#include <string>
#include "Server.hpp"

int	parsePort(const char* str);
bool isPortValid(const char* str);
bool isStrBlank(const char *str);


int main(int argc, char** argv) {
//    int				port;
    std::string				pass;
	Server*	server;

    if (argc != 3 || !isPortValid(argv[1]) || isStrBlank(argv[2])) {
        std::cerr << "./ircserv <port (unsigned short)> <password>" << std::endl;
		return (-1);
    }
	pass.append(argv[2]);
	server = new Server(argv[1]);
	try {
		server->startServer();
	} catch (Server::LaunchFailed& ex) {
		std::cerr << ex.what() << std::endl;
	}
    return 0;
}
