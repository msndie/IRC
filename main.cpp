#include <iostream>
#include <string>
#include "Server.hpp"
#include "utils/utils.h"

int main(int argc, char** argv) {
//    int				port;
    std::string	pass;
	Server*		server;

    if (argc != 3 || !isPortValid(argv[1]) || isStrBlank(argv[2])) {
        std::cerr << "./ircserv <port (unsigned short)> <password>" << std::endl;
		return (-1);
    }
	pass.append(argv[2]);
	server = new Server(argv[1], pass.c_str());
	try {
		server->startServer();
	} catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		return (1);
	}
    return 0;
}
