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
	if (argc == 2)
	{
		try
		{
			ConfigParser configs(argv[1]);
			ServerManager manager(configs.get_configs());
		} 
		catch (std::exception &e)
		{
			std::cerr << RED << e.what() << RESET << std::endl;
			return EXIT_FAILURE;
		}
	}
	else
	{
		try
		{
			ConfigParser configs;
			ServerManager manager(configs.get_configs());
		}
		catch (std::exception &e)
		{
			std::cerr << RED << e.what() << RESET << std::endl;
			return EXIT_FAILURE;
		}
	}
    return EXIT_SUCCESS;
}