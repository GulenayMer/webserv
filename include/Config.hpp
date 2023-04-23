#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <arpa/inet.h>
#include <vector>
#include <map>
#include <iostream>
# include <stdlib.h>
#include "Location.hpp"
#include "minilib.hpp"
#include "Utils.hpp"

class Config {
	private:
		size_t 								_client_max_body_size;
		bool								_autoindex;
		uint16_t							_port;
		in_addr								_addr;
		std::string							_server_name;
		std::string							_name_port;
		std::string							_host;
		std::string							_root;
		std::string							_index;
		std::string							_upload_store;
		std::map<int, std::string>			_default_error;
		std::map<std::string, Location>		_location;
		std::map<std::string, std::string>	_redirection;
		int									_error_code;
		std::map<std::string, std::string>	_intr_paths;

	public:
		Config();
		Config(const Config& obj);
		~Config();
		Config& operator=(const Config& obj);
		void init();

		// getters
		uint16_t							&get_port();
		in_addr								&get_addr();
		std::string							&getHost();
		std::string							&getNamePort();
		std::string							&get_server_name();
		std::map<int, std::string>			&get_default_error();
		std::string							&get_error_path(int error);
		size_t 								&get_client_max_body_size();
		bool								&get_autoindex();
		std::string							&get_root();
		std::string							&get_upload_store();
		std::string							get_index();
		std::map<std::string, Location>		&get_location();
		std::map<std::string, std::string>	&getRedirection();
		int									get_error_code();
		std::map<std::string, std::string>	&getIntrPath();

	
		// setters
		void					set_error_code(int error_code);
		void					set_port(uint16_t port);
		void					set_addr(in_addr_t addr);
		void					setHost(std::string host);
		void					setNamePort(std::string name_port);
		void					set_server_name(std::string server_name);
		void					set_default_error(int i, std::string default_error);
		void 					set_client_max_body_size(int clien_max_body_size);
		void					set_autoindex(bool autoindex);
		void					set_root(std::string root);
		void					set_upload_store(std::string upload_store);
		void					set_index(std::string index);
		void					set_location(std::ifstream& config_file, std::string line);
		void					setIntrPath(std::string &ext, std::string &path);
		void					create_default_errors();
		void					check_config();	
};

#endif
