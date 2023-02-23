#ifndef CONFIGPARSER_HPP
# define CONFIGPARSER_HPP

# include <string>
# include <sstream>
# include <iostream>
# include <fstream>
# include <stdlib.h>
# include <string_view>
# include "Config.hpp"
# include "Utils.hpp"

# define LISTEN "listen"
# define HOST "host"
# define SERVER_NAME "server_name"
# define ERROR_PAGE	"error_page"
# define ALLOW_METHODS "allow_methods"
# define CLIENT_MAX_BODY "client_max_body_size"									  
# define ROOT "root"
# define INDEX "index"
# define ALIAS "alias"
# define AUTOINDEX "autoindex"
# define RETURN "return"
# define CGI_EXT "cgi_ext"
# define CGI_PATH "cgi_path"

class ConfigParser {
	private:
		std::vector<Config> _config;
		int					_n_servers;
		int					_error_code;

	public:
		ConfigParser();
		ConfigParser(std::string config_file);
		

		// Getter
		Config get_config();
		int get_error_code();

		// Cleaners
		int clean_listen(std::string line);
		int clean_host(std::string line);
		int clean_error_page(std::string line);
		int clean_server_name(std::string line);
		int clean_client_max_body_size(std::string line);
		int clean_autoindex(std::string line);
		int clean_root(std::string line);
		int clean_index(std::string line);
		int clean_location(std::ifstream& config_file, std::string line);

		// Utils
		std::string remove_comments(std::string line);
		std::string remove_end(std::string line, char symbol);
		std::string find_int(std::string line);
		std::string get_value(std::string line);
		int    		toInt(std::string str);



		// Error Handling
		int exit_with_error(int err_code, std::ifstream& in_file);
		bool check_server_context(std::ifstream& config_file);
};
		


#endif