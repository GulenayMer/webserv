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

class ConfigParser {
	private:
		int							_n_servers;
		int							_error_code;
		std::vector<std::string>	_path;
		std::vector<std::string>	_ext;

	public:
		std::vector<Config> _configs;
		ConfigParser();
		ConfigParser(std::string config_file);
		~ConfigParser();
		
		// Getter
		std::vector<Config>	&get_configs();
		Config& get_config(int i);
		int get_error_code();
		int get_n_servers();

		//Setters
		void set_error_code(int error_code);
		void set_n_servers(int i);

		// Cleaners
		void clean_listen(std::string line);
		void clean_host(std::string line);
		void clean_error_page(std::string line);
		void clean_server_name(std::string line);
		void clean_client_max_body_size(std::string line);
		void clean_autoindex(std::string line);
		void clean_root(std::string line);
		void clean_index(std::string line);
		void clean_path(std::string line);
		void clean_ext(std::string line);
		void clean_upload_store(std::string line);
		void addToExtMap();
		std::string findIP(std::string &word);

		// Error Handling
		int exit_with_error(int err_code, std::ifstream& in_file);
		bool check_server_context(std::ifstream& config_file);
};
		
#endif
