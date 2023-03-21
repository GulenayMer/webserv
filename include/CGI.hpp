#ifndef CGI_HPP
# define CGI_HPP

# include "Utils.hpp"
# include "Config.hpp"

class CGI {
	private:
		httpHeader							_request;
		std::string							_response_body;
		Config								_config;
		char**								_exec_env;
		std::map<std::string, std::string>	_env;
		
	public:
		CGI(const CGI& obj);
		CGI(Config config, httpHeader request, std::string response_body);
		CGI& operator=(const CGI& obj);
		~CGI();

		// getters
		std::string	get_response_body();

		void	env_init(void);
		void	env_to_char(void);
		int		handle_cgi();//std::ostringstream &response_stream);
		void	exec_script(int *pipe, std::string path, std::string program);
};



#endif