#include "../include/ServerManager.hpp"

/*
	Server Manager :
	+ gets the config file, initializes the variables from the config, unless it has errors
	+ after that it runs the server according to the incoming request, 
		# this process includes I/O multiplexing, 
		# which allows a single thread/process to handle multiple I/O operations simultaneously.
		# poll() method is used for this, which allows monitoring multiple fds to determine which ones are ready for I/O operations.
		# when fds are ready for the connection, it accepts & connects
		# then sends the responses according to requests
*/

ServerManager::ServerManager(std::vector<Config> &configs): _configs(configs), _nfds(0) {
    _fds = NULL;
	for (size_t i = 0; i < this->_configs.size(); i++)
    {
		try {
			std::cout << BLUE << "[---------- Server : " << i << " ----------]\n" << RESET;
			this->_configs[i].check_config();
			std::cout << "name : " << this->_configs[i].get_server_name() << "\n";
			std::cout << "root : " << this->_configs[i].get_root() << "\n";
			std::cout << "host : " << this->_configs[i].getHost() << "\n\n";
			Server server(this->_configs[i]);
			std::map<std::string, std::string>::iterator it = this->_default_host.find(this->_configs[i].getHost());
			if (it == this->_default_host.end())
			{
				server.initServer();
				this->_default_host.insert(std::map<std::string, std::string>::value_type(this->_configs[i].getHost(), this->_configs[i].getNamePort()));
			}
			this->_host_serv.insert(std::map<std::string, Server>::value_type(this->_configs[i].getNamePort(), server));
		}
		catch (std::logic_error &e) {
			server_create_error(e, i);
		}
    }
	if (this->_host_serv.size() > 0) 
	{
		this->_compress_array = false;
		this->_fds = new struct pollfd[FD_SETSIZE];
		memset(this->_fds, 0, FD_SETSIZE * sizeof(struct pollfd));
		for (int i = 0; i < FD_SETSIZE; i++)
			this->_fds[i].fd = -1;
		this->_nfds = 0;
		this->pollfd_init();
		this->_n_servers = this->_nfds;
		this->run_servers();
	}
	else
		std::cerr << RED << "ERROR: --- No valid configurations provided to create servers ---\n" << RESET;
}

ServerManager::~ServerManager()
{
	if (this->_fds)
		delete [] this->_fds;
}

int ServerManager::pollfd_init()
{
	int	i = 0;
	std::map<std::string, Server>::iterator it = this->_host_serv.begin();
	for (; it != this->_host_serv.end(); it++)
    {
		int j = 0;
		for (; j < i; j++)
		{
			if (this->_fds[j].fd == it->second.get_sockfd())
				break;
		}
		if (j == i)
		{
			this->_fds[i].fd = it->second.get_sockfd();
			this->_fds[i].events = POLLIN;
			this->_nfds++;
		}
		i++;
    }
	return i;
}

int ServerManager::compress_arrary()
{
	this->_compress_array = false;
	int count = 0;
	for (int i = this->_n_servers; i < this->_nfds; i++)
	{
		if (this->_fds[i].fd == -1)
		{
			count++;
			if (i == this->_nfds - 1)
			{
				this->_fds[i].events = 0;
				break ;
			}
			for (int j = i + 1; j < this->_nfds; j++)
			{
				if (this->_fds[j].fd != -1)
				{
					this->_fds[i].fd = this->_fds[j].fd;
					this->_fds[i].events = this->_fds[j].events;
					this->_fds[j].fd = -1;
					this->_fds[j].events = 0;
					break ;
				}
				if (j == this->_nfds - 1)
				{
					this->_fds[i].events = 0;
				}
			}
		}
	}
	this->_nfds -= count;
	return count;
}

int ServerManager::run_servers()
{
	int	nbr_fd_ready;
	int	connection_fd;
	while (SWITCH)
    {
        compress_arrary();
		// std::cout << "number of fds: " << this->_nfds << "\n";
		// for (int i = 0; i < FD_SETSIZE; i++)
		// {
		// 	if (this->_fds[i].fd != -1)
		// 		std::cout << "fd: " << this->_fds[i].fd << "\n";
		// }
		nbr_fd_ready = poll(this->_fds, this->_nfds, -1);
        if (nbr_fd_ready == -1)
        {
			perror("poll");
			continue ;
        }
		for (int i = 0; i < this->_nfds; i++)
		{
			if (this->_fds[i].revents == 0)
				continue ;
			if (i < this->_n_servers)
			{
				struct sockaddr_in addr;
				socklen_t addr_len = sizeof(sockaddr_in);
				connection_fd = accept(this->_fds[i].fd, (struct sockaddr *)&addr, &addr_len);
				if (connection_fd < 0)
					perror("accept");
				if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1)
				{
					perror("fcntl set_flags");
					close(connection_fd);
				}
				std::string address(inet_ntoa(addr.sin_addr));
				this->_fds[this->_nfds].fd = connection_fd;
				this->_fds[this->_nfds].events = POLLIN;
				this->_responses.insert(std::map<int, Response>::value_type(this->_fds[this->_nfds].fd, Response(this->_fds[this->_nfds].fd, this->_fds[i].fd, this->_fds, this->_nfds, address)));
				this->_nfds++;
			}
			else if (this->_fds[i].revents & POLLIN)
			{
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				if (response_it != this->_responses.end())
				{
					char	buffer[BUFFER_SIZE];
					ssize_t		received;
					memset(buffer, 0, sizeof(buffer));
					received = recv(this->_fds[i].fd, buffer, sizeof(buffer), MSG_DONTWAIT);
					if (received < 0)
						perror("recv");
					else if (received == 0)
					{
						std::cout << "Received 0, closing connection." << std::endl;
						this->close_connection(response_it->second, i);
					}
					else
					{
						std::cout << "buffer:\n" << buffer << "end of buffer." << std::endl;
						/* [ prepare response ] */
						std::map<int, CGI>::iterator cgi_it = this->_cgis.find(response_it->second.getCGIFd());
						if (cgi_it != this->_cgis.end() && !cgi_it->second.completeContent()) // cgi fd
						{
							std::cout << "adding new received data to cgi buffer" << std::endl;
							if (cgi_it->second.getResponse().isChunked())
								cgi_it->second.mergeChunk(buffer, received);
							else
								cgi_it->second.storeBuffer(buffer, received);
						}
						else //no ongoing cgi
						{
							std::cout << "new request" << std::endl;
							httpHeader request(buffer);
							std::string host(request.get_single_header("host"));
							std::map<std::string, Server>::iterator serv_it = this->_host_serv.find(host);
							if (serv_it != this->_host_serv.end())
							{
								std::cout << serv_it->second.get_config().getNamePort() << std::endl;
								response_it->second.newConfig(serv_it->second.get_config());
							}
							else
							{
								std::map<std::string, std::string>::iterator def_it = this->_default_host.find(host);
								if (def_it != this->_default_host.end())
								{
									std::cout << BLUE << "Using default server for IP/Port: " << def_it->second << "\n" << RESET;
									response_it->second.newConfig(this->_host_serv.find(def_it->second)->second.get_config());
								}
								else
								{
									std::cout << BLUE << "Finding default server for port\n" << RESET;
									host = getDefPort(host);
									if (host.empty())
									{
										std::cout << RED << "No default server found for port. Request from: " << request.get_single_header("host") << "\n" << RESET;
										close_connection(response_it->second, i);
										continue;
									}
									std::cout << BLUE << "Using default server for port: " << host << RESET << std::endl;
									response_it->second.newConfig(this->_host_serv.find(host)->second.get_config());
								}
							}
							response_it->second.new_request(request);
							response_it->second.handle_response();
							response_it->second.getRequest().setStatusCode(get_cgi_response(response_it->second.get_response()));
							if (response_it->second.is_cgi() == false)
								response_it->second.getRequest().printHeader();
							if (response_it->second.shouldClose())
								close_connection(response_it->second, i);
							else if (response_it->second.is_cgi()) // initialise cgi process
							{
								if (this->initCGI(response_it->second, buffer, received, i, request))
								{
									this->_fds[i].events = POLLIN | POLLOUT;
									response_it->second.setCompletion(false);
								}
							}
						}
					}
				}
				else // CGI out ready for reading
				{
					char	buffer[BUFFER_SIZE];
					memset(buffer, 0, BUFFER_SIZE);
					std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
					ssize_t rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
					if (rec > 0)
						cgi_it->second.add_to_buffer(buffer, rec);
					else if (rec < 0)
						perror("read");
				}
			}
			if (this->_fds[i].revents & POLLHUP) // if CGI and CGI out remote end closed -> read remaining to internal buffer, close local end
			{
				std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
				if (cgi_it != this->_cgis.end())
				{
					char	buffer[BUFFER_SIZE];
					ssize_t	rec;
					memset(buffer, 0, BUFFER_SIZE);
					rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
					while (rec > 0)
					{
						cgi_it->second.add_to_buffer(buffer, rec);
						rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
					}
					if (rec == 0)
						cgi_it->second.setReadComplete();
					else
						std::cout << RED << "terrible news\n" << RESET;
					close(this->_fds[i].fd);
					this->_fds[i].fd = -1;
					this->_compress_array = true;
				}
				else
					std::cout << RED << "something just went down\n" << RESET;
			}
			if (this->_fds[i].revents & POLLOUT && this->_fds[i].fd > 0) // if POLLOUT -> write to fd ready for writing
			{
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				std::map<int, CGI>::iterator cgi_it = this->_cgis.end();
				std::map<int, int>::iterator cgi_fd_it = this->_cgi_fds.find(this->_fds[i].fd);
				if (response_it != this->_responses.end())
					cgi_it = this->_cgis.find(response_it->second.getCGIFd());
				if (response_it != this->_responses.end() && !response_it->second.is_cgi()) // for sending non-CGI responses
				{
					std::cout << "SEND RESPONSE\n";
					response_it->second.handle_response();
					response_it->second.getRequest().printHeader();
					if (response_it->second.shouldClose())
						close_connection(response_it->second, i);
					else if (response_it->second.response_complete())
						this->_fds[i].events = POLLIN;
				}
				else if (cgi_it != this->_cgis.end() && cgi_it->second.readComplete()) // if done reading from cgi out, send response to client
				{
					if (cgi_it->second.sendResponse())
					{
						cgi_it->second.getResponse().getRequest().setStatusCode(get_cgi_response(cgi_it->second.get_response_string()));
						cgi_it->second.getResponse().getRequest().setSentSize(cgi_it->second.get_size_sent());
						cgi_it->second.getResponse().getRequest().printHeader();
						this->_cgis.erase(cgi_it);
						this->_compress_array = true;
						this->_fds[i].events = POLLIN;
					}
				}
				else if (cgi_fd_it != this->_cgi_fds.end()) // write to cgi stdin
				{
					cgi_it = this->_cgis.find(cgi_fd_it->second);
					if (!cgi_it->second.bodySentCGI())
					{
						if (!cgi_it->second.getResponse().isChunked())
						{
							std::cout << "SEND BODY TO CGI\n";
							cgi_it->second.writeToCGI();
						}
					}
					else if (cgi_it->second.bodySentCGI())
					{
						std::cout << "CGI BODY COMPLETE\n";
						close(this->_fds[i].fd);
						this->_fds[i].fd = -1;
						this->_compress_array = true;
						this->_cgi_fds.erase(cgi_fd_it);
					}
				}
			}
			nbr_fd_ready--;
			if (nbr_fd_ready == 0)
				break ;
		}
		if (this->_compress_array)
		{
			// compress_arrary();
			// this->_compress_array = false;
			// /* go through all the fd array and move all the fds that are not -1 to the front of the array */
			// int count = 0;
			// for (int i = this->_n_servers; i < this->_nfds; i++)
			// {
			// 	if (this->_fds[i].fd == -1)
			// 	{
			// 		if (i == this->_nfds - 1)
			// 		{
			// 			this->_fds[i].events = 0;
			// 			break ;
			// 		}
			// 		for (int j = i + 1; j < this->_nfds; j++)
			// 		{
			// 			if (this->_fds[j].fd != -1)
			// 			{
			// 				this->_fds[i].fd = this->_fds[j].fd;
			// 				this->_fds[i].events = this->_fds[j].events;
			// 				this->_fds[j].fd = -1;
			// 				this->_fds[j].events = 0;
			// 				break ;
			// 			}
			// 			if (j == this->_nfds - 1)
			// 			{
			// 				this->_fds[i].events = 0;
			// 			}
			// 		}
			// 		count++;
			// 	}
			// }
			// this->_nfds -= count;
			// for (int i = this->_nfds - 1; i >= this->_n_servers; i--)
			// {
			// 	if (this->_fds[i].fd == -1)
			// 	{
			// 		for (int j = this->_nfds - 1; j > i; j--)
			// 		{
			// 			if (this->_fds[j].fd != -1)
			// 			{
			// 				this->_fds[i].fd = this->_fds[j].fd;
			// 				this->_fds[i].events = this->_fds[j].events;
			// 				this->_fds[j].fd = -1;
			// 				this->_fds[j].events = 0;	
			// 				break ;
			// 			}
			// 		}
			// 		this->_nfds--;
			// 	}
			// }
		}
    }
	for (int i = 0; i < this->_nfds; i++)
	{
		if (this->_fds[i].fd > 0)
		{
			close(this->_fds[i].fd);
			this->_fds[i].fd = -1;
		}
	}
	return EXIT_SUCCESS;
}

// std::vector<Server>	ServerManager::get_servers()
// {
// 	return this->_servers;
// }

// Server	ServerManager::get_server_at(int i)
// {
// 	return this->_servers[i];
// }

void	ServerManager::close_connection(Response &response, int i)
{
	//std::cout << "closing conn fd" << std::endl;
	this->_addr_fd.erase(response.getAddress());
	this->_responses.erase(this->_fds[i].fd);
	if (this->_fds[i].fd > 0)
	{
		close(this->_fds[i].fd);
		this->_fds[i].fd = -1;
		this->_compress_array = true;
	}
}

bool	ServerManager::initCGI(Response &response, char *buffer, ssize_t received, int i, httpHeader &request)
{
	CGI cgi(response, request);
	int out_fd = cgi.initOutputPipe();
	int in_fd = cgi.initInputPipe();
	if (out_fd < 0 || in_fd < 0)
	{
		cgi.sendResponse();
		close_connection(cgi.getResponse(), i);
		return false;
	}
	else
	{
		std::pair<std::map<int, CGI>::iterator, bool> ret_pair = this->_cgis.insert(std::map<int, CGI>::value_type(out_fd, cgi));
		std::cout << RED << "Could insert into CGI map: " << ret_pair.second << "\n";
		std::pair<std::map<int, int>::iterator, bool> cgi_ret_pair = this->_cgi_fds.insert(std::map<int, int>::value_type(in_fd, out_fd));
		std::cout << "Could insert into CGI-FD map: " << cgi_ret_pair.second << RESET << std::endl;
		std::map<int, CGI>::iterator cgi_it = ret_pair.first;
		this->_fds[_nfds].fd = out_fd;
		this->_fds[_nfds].events = POLLIN;
		this->_fds[_nfds].revents = 0;
		_nfds++;
		this->_fds[_nfds].fd = in_fd;
		this->_fds[_nfds].events = POLLOUT;
		this->_fds[_nfds].revents = 0;
		_nfds++;
		if (!cgi_it->second.handle_cgi())
		{
			_nfds--;
			this->_fds[_nfds].fd = -1;
			_nfds--;
			this->_fds[_nfds].fd = -1;
			this->_compress_array = true;
			cgi_it->second.closePipes();
			cgi_it->second.sendResponse();
			cgi_it->second.getResponse().getRequest().printHeader();
			close_connection(cgi_it->second.getResponse(), i);
			this->_cgis.erase(ret_pair.first);
			this->_cgi_fds.erase(cgi_ret_pair.first);
			return false;			
		}
		response.setCGIFd(out_fd);
		if (response.isChunked())
			cgi_it->second.removeHeader(buffer, received);
		else
		{
			cgi_it->second.storeBuffer(buffer, received);
			cgi_it->second.writeToCGI();
		}
	}
	return true;
}

void	ServerManager::server_create_error(std::logic_error &e, int i)
{
	std::cerr << "\n";
	std::cerr << RED << "---------------------------------------------------------------------------\n\n";
	std::cerr << "Could not create server at index: " << i;
	std::cerr << "\nServer name: " << this->_configs[i].get_server_name() << "\n";
	std::cerr << e.what();
	std::cerr << "\n\n---------------------------------------------------------------------------\n\n" << RESET;
}

int		ServerManager::get_cgi_response(std::string header)
{
	if (!header.empty())
	{
		size_t				start, end;
		int					result;

		start = header.find_first_not_of(" \r\t\b\f");
		start = header.find_first_of(" \r\t\b\f", start);
		start = header.find_first_not_of(" \r\t\b\f", start);
		end = header.find_first_of(" \r\t\b\f", start);
		std::stringstream	ss(header.substr(start, end - start));
		ss >> result;

		return result;
	}
	std::cout << RED << "ERROR : something wrong with logs\n" << RESET;
	return (0);
}

std::string	ServerManager::getDefPort(std::string &host)
{
	size_t pos = host.find_first_of(':');
	if (pos != std::string::npos && pos != host.size() - 1)
	{
		host.erase(0, pos);
		std::map<std::string, std::string>::iterator it = this->_default_host.begin();
		while (it != this->_default_host.end())
		{
			pos = it->first.find(host);
			if (pos != std::string::npos && pos == it->first.size() - host.size())
				return it->second;
			it++;
		}
	}
	return "";
}
