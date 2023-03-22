#ifndef CGI_HPP
# define CGI_HPP

# include "Utils.hpp"
# include "Config.hpp"
# include "Response.hpp"
# include <fcntl.h>

class Response;

enum CGIerrors 
{
	SOMEERROR = 1,
	INVALID_REQUEST,
	INCOMPLETE_WRITE,
};

class CGI {
	private:
		Response							_response;
		std::string							_request_body;
		bool								_done_reading;
		bool								_body_complete;
		bool								_header_removed;
		char**								_exec_env;
		std::string							_buffer;
		size_t								_bytes_sent;
		std::map<std::string, std::string>	_env;
		int									_input_pipe[2];
		int									_output_pipe[2];
		std::string							_boundary;
		int									_errnum;
		
	public:
		CGI(Response &response, std::string request_body);
		CGI(const CGI& obj);
		CGI& operator=(const CGI& obj);
		~CGI();

		// getters
		std::string	get_response_body();

		void		env_init(void);
		void		env_to_char(void);
		int			handle_cgi();//std::ostringstream &response_stream);
		void		exec_script(int *input_pipe, int *output_pipe, std::string path, std::string program);
		std::string get_path_from_map();
		std::string get_query();
		std::string& get_boundary();
		int		initPipe();
		Response &getResponse();
		void	sendResponse();
		void	add_to_buffer(char *buff);
		void	setReadComplete();
		bool	readComplete();
		void	fill_in_body(char *buffer);
		void	set_boundary();
		bool	bodyComplete();
};



#endif