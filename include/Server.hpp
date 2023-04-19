#ifndef SERVER_HPP
# define SERVER_HPP

# include "Utils.hpp"
# include "Config.hpp"
# include "MIME.hpp"
# include "minilib.hpp"
# include <sstream>
# include <fstream>
# include <iostream>
# include <string>
# include <cstring>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/wait.h>

class Server
{
	private:
		struct sockaddr_in	_serv_addr;
		Config      	   	_config;
		int 				_sockfd;
    	int 				_port;
		int					_error;
		MIME				_types;

    public:
		Server(Config config);
		// Server(const Server& copy);
		// Server& operator=(const Server& rhs);
		~Server();

		void	initServer();
		int		init_socket();
		int		bind_socket();
		int		listen_socket();
		
		// getters
		
		int     	get_sockfd() const;
        int     	get_port() const;
		int	    	getError()	const;
		
		Config 		&get_config();
		
		// utils
		
		//void	send_response(int client_socket, const std::string& path);
		int		clean_fd();
		//int		handle_cgi(const std::string& path, std::string& response_body);
		//void	exec_script(int *pipe, std::string path, std::string program);
		//void	send_404(std::string root, std::ostringstream &response_stream);
		std::string contentType(int flag);
};

#endif