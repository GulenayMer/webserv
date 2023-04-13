#include <stdio.h> // we will not need it probably
#include "../include/Utils.hpp"
#include "../include/httpHeader.hpp"
#include "../include/ConfigParser.hpp"
#include "../include/ServerManager.hpp"
#include "../include/minilib.hpp"

int main(int argc, char** argv)
{
    if (argc > 2) {
		std::cerr << RED << TOO_MANY_ARGS << RESET << std::endl;
		return EXIT_FAILURE;
	}
	signal(SIGINT, signal_callback_handler);
	signal(SIGTSTP, signal_callback_handler);
	signal(SIGCHLD, grim_reaper);
	if (argc == 2) {
		ConfigParser configs(argv[1]);
		if (configs.get_error_code() != 0)
			return EXIT_FAILURE;
		ServerManager manager(configs.get_configs());
	}
	else
	{
		ConfigParser configs;
		ServerManager manager(configs.get_configs());
	}
    return EXIT_SUCCESS;
}