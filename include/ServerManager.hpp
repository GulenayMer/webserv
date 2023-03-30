#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <poll.h>
# include "Server.hpp"
# include "Config.hpp"
# include "Response.hpp"
# include "CGI.hpp"

class ServerManager {
	
    private:
		std::vector<Server> _servers;
		std::vector<Config> _configs;
		std::map<int, Response> _responses;
		std::map<int, CGI>		_cgis;
		std::map<int, int>		_cgi_fds;
		std::map<std::string, int>	_addr_fd;
		int					_nfds;
		struct pollfd*		_fds;
		int					_nbr_fd_ready;
		std::map<int, int>	_map_server_fd;
		bool				_compress_array;
        // 	ServerManager(ServerManager const &copy);
        //  ServerManager &operator=(ServerManager const &rhs);
    
    public:
		ServerManager(std::vector<Config> configs);

        ~ServerManager();

		void 	pollfd_init();
		int		run_servers();
		int		check_connection();
		int		check_request_respond();
		void	close_connection(std::map<int, Response>::iterator it, int i);

		std::vector<Server>	get_servers();
		Server				get_server_at(int i);
		bool	initCGI(Response &response, char *buffer, ssize_t received);
};

#endif