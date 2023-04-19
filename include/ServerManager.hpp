#ifndef SERVERMANAGER_HPP
# define SERVERMANAGER_HPP

# include <poll.h>
# include "Server.hpp"
# include "Config.hpp"
# include "Response.hpp"
# include "CGI.hpp"
# include "iomanip"

class ServerManager {
	
    private:
		std::map<std::string, Server> _host_serv;
		int	_n_servers;
		std::vector<Config> _configs;
		std::map<int, Response> _responses;
		std::map<int, CGI>		_cgis;
		std::map<int, int>		_cgi_fds;
		std::map<std::string, int>	_addr_fd;
		std::map<std::string, std::string> _default_host;
		int					_nfds;
		struct pollfd*		_fds;
		int					_nbr_fd_ready;
		std::map<int, int>	_map_server_fd;
		bool				_compress_array;
        // 	ServerManager(ServerManager const &copy);
        //  ServerManager &operator=(ServerManager const &rhs);
    
    public:
		ServerManager(std::vector<Config> &configs);

        //~ServerManager();

		int 	pollfd_init();
		int		run_servers();
		//int		check_connection();
		//int		check_request_respond();
		void	close_connection(Response &response, int i);

		std::vector<Server>	get_servers();
		Server				get_server_at(int i);
		bool	initCGI(Response &response, char *buffer, ssize_t received, int i, httpHeader *request);

		void	server_create_error(std::logic_error &e, int i);
		int		get_cgi_response(std::string header);
};

#endif