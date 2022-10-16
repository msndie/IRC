#include <iostream>
#include <string>
#include "../includes/Server.hpp"

int main(int argc, char** argv) {
	int			status = 0;

    if (argc != 2 || isStrBlank(argv[1])) {
        std::cerr << "./bot <config path>" << std::endl;
		return (-1);
    }

    return status;
}
