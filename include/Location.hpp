#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <string>
# include <map>
# include <iostream>
# include "Utils.hpp"

	class Location {
		private:
			// Data to send
			std::string						_root;          	
      		bool							_autoindex;
			bool							_allow_cgi;
     		std::map<short, bool> 			_methods;
      		std::string						_index;
      		std::string						_redirection;
     	 	std::string						_alias;
			int								_error_code;    

		public:

			Location();
			Location(const Location &src);
			Location &operator=(const Location &src);
			Location(std::ifstream &config_file, std::string line, std::string config_root,std::string config_index, std::string key);
			// Setters   
			void set_root(std::string root);
			void set_autoindex(bool autoindex);
			void set_methods(short method, bool allowed);
			void set_index(std::string index);
			void set_redirection(std::string redirection);
			void set_alias(std::string alias);
			void set_error_code(int error_code);
			void set_allow_cgi(std::string allow_cgi);
		
			// Getters
			std::string						get_root();          	
      		bool							get_autoindex();
			std::map<short, bool>			get_methods();
      		std::string						get_index();
      		std::string						get_redirection();
     	 	std::string						get_alias();
			int 							get_error_code();
			bool							allow_cgi();

			bool							check_method_at(short method);
			int								check_location();
			bool 							clean_loc_autoindex(std::string line);
			void 							clean_methods(std::string line);
			void							init_methods();
		

	};
	std::ostream& operator<<(std::ostream& os, Location& location);
#endif