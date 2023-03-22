#ifndef CGI_HPP
# define CGI_HPP

# include "Utils.hpp"
# include "Config.hpp"
# include "Response.hpp"
# include <fcntl.h>

class Response;

class CGI {
	private:
		Response							_response;
		bool								_done_reading;
		char**								_exec_env;
		std::string							_buffer;
		size_t								_bytes_sent;
		std::map<std::string, std::string>	_env;
		int									_pipe[2];
		
	public:
		CGI(Response &response);
		CGI(const CGI& obj);
		CGI& operator=(const CGI& obj);
		~CGI();

<<<<<<< HEAD
		// getters
		std::string	get_response_body();

		void		env_init(void);
		void		env_to_char(void);
		int			handle_cgi();//std::ostringstream &response_stream);
		void		exec_script(int *pipe, std::string path, std::string program);
		std::string get_path_from_map();
		std::string get_query();
=======
		void	env_init(void);
		void	env_to_char(void);
		int		handle_cgi();
		void	exec_script(int *pipe, std::string path, std::string program);
		int		initPipe();
		Response &getResponse();
		void	sendResponse();
		void	add_to_buffer(char *buff);
		void	readComplete();
		bool	doneReading();
>>>>>>> 0d400c917beae7161290cbc2f1054d86c68bbdd6
};



#endif