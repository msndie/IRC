#include <iostream>
#include "../includes/utils.h"
#include "../includes/Bot.hpp"

int main(int argc, char** argv) {
	int	status = 0;
	Bot	*bot;

    if (argc != 2 || isStrBlank(argv[1])) {
        std::cerr << "./bot <config path>" << std::endl;
		return (-1);
    }

	bot = new Bot(argv[1]);

	try {
		bot->startBot();
	} catch (std::exception& ex) {
		std::cerr << ex.what() << std::endl;
		status = 1;
	}

	delete bot;

    return status;
}
