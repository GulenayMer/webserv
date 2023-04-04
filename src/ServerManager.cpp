#include "../include/ServerManager.hpp"

ServerManager::ServerManager(std::vector<Config> configs): _configs(configs), _nfds(0) {


    for (size_t i = 0; i < this->_configs.size(); i++)
    {
		try {
			this->_configs[i].check_config();
        	Server server = Server(this->_configs[i]);
       		this->_servers.push_back(server);
		}
		catch (std::logic_error &e) {
			std::cerr << std::endl;
			std::cerr << RED << "---------------------------------------------------------------------------" << std::endl;
			std::cerr << std::endl;
			std::cerr << "Could not create server at index: " << i  << std::endl;
			std::cerr << "Server name: " << this->_configs[i].get_server_name()  << std::endl;
			std::cerr << e.what() << std::endl;
			std::cerr << std::endl;
			std::cerr << "---------------------------------------------------------------------------" << RESET << std::endl;
			std::cerr << std::endl;
		}
    }
	if (this->get_servers().size() > 0) {
		this->_compress_array = false;
		this->_fds = new struct pollfd[MAX_CONN * this->get_servers().size()];
		this->pollfd_init();
		this->_nfds = this->get_servers().size();
		this->run_servers();
	}
	else
		std::cerr << RED << NO_VALID_SERVERS << RESET << std::endl;
}

ServerManager::~ServerManager()
{
	for (size_t i = 0; i < this->get_servers().size(); i++)
		this->get_server_at(i).clean_fd();
	if (this->get_servers().size() > 0) {
		delete [] this->_fds;
	}
}

void ServerManager::pollfd_init()
{
	for (size_t i = 0; i < this->_configs.size(); i++)
    {
        this->_fds[i].fd = this->_servers[i].get_sockfd();
        this->_fds[i].events = POLLIN;
    }
}

int ServerManager::run_servers()
{
	// This whole thing probably has memory leaks
	int		nbr_fd_ready;
	while (SWITCH)
    {
        nbr_fd_ready = poll(this->_fds, this->_nfds, -1);
        if (nbr_fd_ready == -1)
        {
            // TODO avoid killing the server, implement test how to deal with it 
			perror("poll");
			continue ;
        }
		//std::cout << RED << "NUMBER OF FDS: " << this->_nfds << RESET << std::endl;
		for (int i = 0; i < this->_nfds; i++)
		{
			//std::cout << RED << "FD: " << this->_fds[i].fd << " FD NO: " << i << RESET << std::endl;
			if (this->_fds[i].revents == 0)
				continue ;
			if (i < (int)this->_servers.size())
			{
				sockaddr addr;
				socklen_t addr_len = 0;
				int	connection_fd;
				connection_fd = accept(this->_servers[i].get_sockfd(), &addr, &addr_len);
				struct sockaddr_in *s = (struct sockaddr_in *)&addr;
				std::string address(inet_ntoa(s->sin_addr));
				// if (this->_addr_fd.find(address) != this->_addr_fd.end())
				// {
				// 	close(connection_fd);
				// 	continue;
				// }
				std::cout << "address: " << address << std::endl;
				// std::cout << "address data: " << addr.sa_data << std::endl;
				std::cout << "address length: " << addr_len << std::endl;
				this->_addr_fd.insert(std::map<std::string, int>::value_type(address, connection_fd));
				if (connection_fd < 0)
				{
					perror("accept");
					nbr_fd_ready--;
					continue ;
                }
				if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1)
				{
					perror("fcntl set_flags");
					close(connection_fd);
					continue ;
				}
				std::cout << GREEN << "New Connection" << RESET << std::endl;
				this->_fds[this->_nfds].fd = connection_fd;
				this->_fds[this->_nfds].events = POLLIN;
				this->_responses.insert(std::map<int, Response>::value_type(this->_fds[this->_nfds].fd, Response(this->_fds[this->_nfds].fd, this->_servers[i].get_sockfd(), this->_servers[i].get_config(), this->_fds, this->_nfds, address)));
				this->_nfds++;
			}
			else if (this->_fds[i].revents & POLLIN)
			{
				std::cout << "POLLIN" << std::endl;
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				if (response_it != this->_responses.end())
				{
					// receive request ->
					char	buffer[2048];
					//TODO implement client max body size
					ssize_t		received;
					memset(buffer, 0, sizeof(buffer));
					received = recv(this->_fds[i].fd, buffer, sizeof(buffer), MSG_DONTWAIT);
					for (ssize_t l = 0; l < received; l++)
						std::cout << GREEN << buffer[l];
					std::cout << RESET << std::endl;
					std::cout << "REQUEST FD: " << this->_fds[i].fd << std::endl;
					if (received < 0)
					{
						this->close_connection(response_it->second, i);
						perror("recv");
					}
					else if (received == 0)
					{
						this->close_connection(response_it->second, i);
						printf("Connection closed\n");
					}
					else
					{
						std::cout << "RECEIVED: " << received << std::endl;
						/* [ prepare response ] */
						std::map<int, CGI>::iterator cgi_it = this->_cgis.find(response_it->second.getCGIFd());
						//TODO kill CGI process, reset and process new request
						if (cgi_it != this->_cgis.end())
						{
							ssize_t remaining = response_it->second.receivedBytes(received);
							std::cout << "remaining: " << remaining << std::endl;
							if (remaining < 0)
							{
								std::cout << "BUFFER:" << std::endl;
								std::cout << buffer << std::endl;
								kill(cgi_it->second.PID(), SIGTERM);
								for (int j = _nfds - 1; j >= 0; j--)
								{
									if (this->_fds[j].fd == cgi_it->second.getInFd())
									{
										this->_fds[j].fd = -1;
										_nfds -= 1;
									}
									else if (this->_fds[j].fd == cgi_it->second.getOutFd())
									{
										this->_fds[j].fd = -1;
										_nfds -= 1;
									}
								}
								cgi_it->second.closePipes();
								this->_cgis.erase(cgi_it);
								cgi_it = this->_cgis.end();
								this->_compress_array = true;
								remaining = abs(remaining);
								memmove(buffer, &buffer[received - remaining], remaining);
								memset(&buffer[remaining], 0, received - remaining);
							}
						}
						if (cgi_it != this->_cgis.end() && !cgi_it->second.completeContent()) // cgi fd
						{
							cgi_it->second.storeBuffer(buffer, received);
							cgi_it->second.writeToCGI();
						}
						else //not a cgi fd
						{
							std::cout << "creating new response?" << std::endl;
							if (cgi_it != this->_cgis.end())
							{
								kill(cgi_it->second.PID(), SIGTERM);
								for (int j = _nfds - 1; j >= 0; j--)
								{
									if (this->_fds[j].fd == cgi_it->second.getInFd())
									{
										this->_fds[j].fd = -1;
										_nfds -= 1;
									}
									else if (this->_fds[j].fd == cgi_it->second.getOutFd())
									{
										this->_fds[j].fd = -1;
										_nfds -= 1;
									}
								}
								cgi_it->second.closePipes();
								this->_cgis.erase(cgi_it);
								this->_compress_array = true;
							}
							httpHeader request(buffer);
							response_it->second.new_request(request);
							request.printHeader();
							response_it->second.send_response();
							if (response_it->second.shouldClose())
								close_connection(response_it->second, i);
							else if (response_it->second.is_cgi()) // init cgi
							{
								if (this->initCGI(response_it->second, buffer, received, i))
								{
									this->_fds[i].events = POLLIN | POLLOUT;
									response_it->second.completeProg(false);
								}
								else {
									// FAILED initCGI comes out here after error 500
								}
							}
						}
					}
				}
				else // CGI out ready for reading
				{
					std::cout << "CGI PIPE END" << std::endl;
					char	buffer[2048];
					memset(buffer, 0, 2048);
					std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
					ssize_t rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
					if (rec > 0)
						cgi_it->second.add_to_buffer(buffer, rec);
					else if (rec < 0)
						perror("read");
				}
			}
			if (this->_fds[i].revents & POLLHUP && this->_cgis.find(this->_fds[i].fd) != this->_cgis.end()) // if CGI and CGI out remote end closed -> read remaining to internal buffer, close local end
			{
				std::cout << "POLLHUP" << std::endl;
				char	buffer[2048];
				memset(buffer, 0, 2048);
				std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
				while (size_t rec = read(this->_fds[i].fd, buffer, sizeof(buffer)) > 0)
					cgi_it->second.add_to_buffer(buffer, rec);
				cgi_it->second.setReadComplete();
			}
			if (this->_fds[i].revents & POLLOUT && this->_fds[i].fd > 0) // if POLLOUT -> write to fd ready for writing
			{
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				std::map<int, CGI>::iterator cgi_it = this->_cgis.end();
				if (response_it != this->_responses.end())
					cgi_it = this->_cgis.find(response_it->second.getCGIFd());
				std::map<int, int>::iterator cgi_fd_it = this->_cgi_fds.find(this->_fds[i].fd);
				if (response_it != this->_responses.end() && !response_it->second.is_cgi()) // for sending non-CGI responses
				{
					std::cout << "SEND RESPONSE" << std::endl;
					response_it->second.send_response();
					if (response_it->second.shouldClose())
						close_connection(response_it->second, i);
					else if (response_it->second.response_complete())
						this->_fds[i].events = POLLIN;
				}
				else if (cgi_it != this->_cgis.end() && cgi_it->second.readComplete()) // if done reading from cgi out, send response to client
				{
					std::cout << "CGI COMPLETE SEND" << std::endl;
					cgi_it->second.sendResponse();
					for (int j = _nfds - 1; j >= 0; j--)
					{
						if (this->_fds[j].fd == cgi_it->second.getInFd())
						{
							this->_fds[j].fd = -1;
							_nfds -= 1;
						}
						else if (this->_fds[j].fd == cgi_it->second.getOutFd())
						{
							this->_fds[j].fd = -1;
							_nfds -= 1;
						}
					}
					cgi_it->second.closePipes();
					this->_cgis.erase(cgi_it);
					this->_compress_array = true;
					this->_fds[i].events = POLLIN;
				}
				else if (cgi_fd_it != this->_cgi_fds.end()) // write to cgi stdin
				{
					cgi_it = this->_cgis.find(cgi_fd_it->second);
					if (!cgi_it->second.bodySentCGI())
						cgi_it->second.writeToCGI();
					else if (cgi_it->second.bodySentCGI())
					{
						std::cout << "CGI BODY COMPLETE" << std::endl;
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
			std::cout << "COMPRESS" << std::endl;
			this->_compress_array = false;
			for (int i = this->_nfds - 1; i > (int)this->_servers.size() - 1; i--)
			{
				if (this->_fds[i].fd == -1)
				{
					for (int j = this->_nfds - 1; j > i; j--)
					{
						if (this->_fds[j].fd != -1)
						{
							this->_fds[i].fd = this->_fds[j].fd;
							this->_fds[i].events = this->_fds[j].events;
							this->_fds[j].fd = -1;
							this->_fds[j].events = 0;	
							break ;
						}
					}
					this->_nfds--;
				}
			}
		}
		// for (int i = 0; i < this->_nfds; i++)
		// 	this->_fds[i].revents = 0;
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

std::vector<Server>	ServerManager::get_servers()
{
	return this->_servers;
}

Server	ServerManager::get_server_at(int i)
{
	return this->_servers[i];
}

void	ServerManager::close_connection(Response &response, int i)
{
	std::cout << "closing conn fd" << std::endl;
	this->_addr_fd.erase(response.getAddress());
	this->_responses.erase(this->_fds[i].fd);
	if (this->_fds[i].fd > 0)
	{
		close(this->_fds[i].fd);
		this->_fds[i].fd = -1;
		this->_compress_array = true;
	}
}

bool	ServerManager::initCGI(Response &response, char *buffer, ssize_t received, int i)
{
	CGI cgi(response);
	int out_fd = cgi.initOutputPipe();
	int in_fd = cgi.initInputPipe();
	if (out_fd < 0 || in_fd < 0)
	{
		std::cout << RED << "pipe: internal server error -> send 500" << RESET << std::endl; //TODO internal server error - 500
		return false;
	}
	else
	{
		std::pair<std::map<int, CGI>::iterator, bool> ret_pair = this->_cgis.insert(std::map<int, CGI>::value_type(out_fd, cgi));
		std::cout << RED << ret_pair.second << std::endl;
		std::pair<std::map<int, int>::iterator, bool> cgi_ret_pair = this->_cgi_fds.insert(std::map<int, int>::value_type(in_fd, out_fd));
		std::cout << cgi_ret_pair.second << RESET << std::endl;
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
			std::cout << RED << "handle_cgi: internal server error -> send 500" << RESET << std::endl; //TODO internal server error - 500
			cgi_it->second.sendResponse();
			close_connection(cgi_it->second.getResponse(), i);
			this->_cgis.erase(ret_pair.first);
			this->_cgi_fds.erase(cgi_ret_pair.first);
			return false;			
		}
		response.setCGIFd(out_fd);
		cgi_it->second.storeBuffer(buffer, received);
		cgi_it->second.writeToCGI();
	}
	return true;
}
