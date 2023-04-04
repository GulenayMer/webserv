#ifndef CONFIG_HPP
# define CONFIG_HPP

#include <arpa/inet.h>
#include <vector>
#include <map>
#include <iostream>
# include <stdlib.h>
#include "Location.hpp"
#include "configCGI.hpp"
#include "minilib.hpp"
#include "Utils.hpp"


	class Config {
		private:
  			size_t 								_client_max_body_size;
			bool								_autoindex;
			uint16_t							_port;
  			in_addr								_host;                  
  			std::string							_server_name;
  			std::string							_root;
  			std::string							_index;
 			std::map<int, std::string>			_default_error;
  			std::map<std::string, Location>		_location;
  			std::map<std::string, std::string>	_redirection;
			configCGI							_cgi;
			int									_error_code;

		public:
			Config();
			Config(const Config& obj);
			~Config();
			Config& operator=(const Config& obj);

			// getters
			u_int16_t							&get_port();
			in_addr								&get_host();                  
  			std::string							&get_server_name();
 			std::map<int, std::string>			&get_default_error();
			std::string							&get_error_path(int error);
  			size_t 								&get_client_max_body_size();
			bool								&get_autoindex();
  			std::string							&get_root();
  			std::string							get_index();
			std::map<std::string, Location>		&get_location();
			// Location							&find_location(std::string location);
			configCGI							&get_cgi();
			std::map<std::string, std::string>	&getRedirection();
			int									get_error_code();

		
			// setters
			void					set_error_code(int error_code);
			void					set_port(u_int16_t port);
			void					set_host(in_addr_t host);                  
  			void					set_server_name(std::string server_name);
 			void					set_default_error(int i, std::string default_error);
  			void 					set_client_max_body_size(int clien_max_body_size);
			void					set_autoindex(bool autoindex);
  			void					set_root(std::string root);
  			void					set_index(std::string index);
  			void					set_location(std::ifstream& config_file, std::string line);
			void					set_cgi(std::ifstream& config_file, std::string line);
			void					create_default_errors();
			void					check_config();

			// Utils

			
	};
	std::ostream& operator<<(std::ostream& os, Config& config);
#endif