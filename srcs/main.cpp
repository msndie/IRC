#include <iostream>
#include <string>
#include "../includes/Server.hpp"

int main(int argc, char** argv) {
    std::string	pass;
	Server*		server;
	int			status = 0;

    if (argc != 3 || !isPortValid(argv[1]) || isStrBlank(argv[2])) {
        std::cerr << "./ircserv <port (unsigned short)> <password>" << std::endl;
		return (-1);
    }
	pass.append(argv[2]);
	server = new Server(argv[1], pass.c_str());
	try {
		server->startServer();
	} catch (std::exception& ex) {
		if (errno != EINTR) {
			std::cerr << ex.what() << std::endl;
			status = 1;
		}
	}
	delete server;
    return status;
}
