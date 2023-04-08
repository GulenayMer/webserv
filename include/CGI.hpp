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
		bool								_done_reading;
		bool								_body_complete;
		bool								_header_removed;
		char*								_exec_env[18];
		std::string							_buffer;
		std::vector<char>					_request_buff;
		std::vector<char>					_response_buff;
		size_t								_vector_pos;
		size_t								_content_length;
		size_t								_header_length;
		size_t								_bytes_sent;
		std::map<std::string, std::string>	_env;
		int									_input_pipe[2];
		int									_output_pipe[2];
		std::string							_boundary;
		int									_errno;
		pid_t								_pid;			

	public:
		CGI(Response &response);
		CGI(const CGI& obj);
		CGI& operator=(const CGI& obj);
		~CGI();

		// getters
		std::string	get_response_body();

		void		env_init(void);
		void		env_to_char(void);
		bool		handle_cgi();
		void		exec_script(int *input_pipe, int *output_pipe, std::string path, std::string intpr_path);
		std::string get_path_from_map();
		std::string get_query();
		std::string& get_boundary();
		int		initOutputPipe();
		int		initInputPipe();
		Response &getResponse();
		bool	sendResponse();
		void	add_to_buffer(char *buff, size_t rec);
		void	setReadComplete();
		bool	readComplete();
		void	storeBuffer(char *buffer, size_t received);
		void	set_boundary();
		bool	bodySentCGI();
		int		getOutFd();
		int		getInFd();
		void	writeToCGI();
		bool	completeContent();
		int		PID();
		void	closePipes();
		void	mergeChunk(char *buffer);
		size_t	convertHex(char *buffer, size_t &size);
		void	addHeaderChunked();
};



#endif