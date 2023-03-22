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
	bool	close_connection = false;
	bool	compress_array = false;
	int		nbr_fd_ready;
	while (SWITCH)
    {
        nbr_fd_ready = poll(this->_fds, this->_nfds, -1);
        if (nbr_fd_ready == -1)
        {
            // TODO avoid killing the server, implement test how to deal with it 
			perror("poll");
            return 1;
        }
		for (int i = 0; i < this->_nfds; i++)
		{
			if (this->_fds[i].revents == 0)
				continue ;
			if (i < (int)this->get_servers().size())
			{
				int	connection_fd;
				connection_fd = accept(this->_servers[i].get_sockfd(), NULL, NULL);
				if (connection_fd < 0)
				{
                    // TODO avoid killing the server, implement test how to deal with it (This fix might work, need to test it out)
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
				this->_responses.insert(std::map<int, Response>::value_type(this->_fds[this->_nfds].fd, Response(this->_fds[this->_nfds].fd, this->_servers[i].get_sockfd(), this->_servers[i].get_config(), this->_fds, this->_nfds)));
				this->_nfds++;
			}
			else if (this->_fds[i].revents & POLLIN)
			{
				close_connection = false;
				if (this->_responses.find(this->_fds[i].fd) != this->_responses.end())
				{
					// receive request ->
					std::map<int, int>::iterator it = this->_map_server_fd.find(i);
					char	buffer[this->_servers[it->second].get_config().get_client_max_body_size()];
					//TODO implement client max body size
					int		received;

					memset(buffer, 0, sizeof(buffer));
					std::cout << this->_fds[i].fd << std::endl;
					memset(buffer, 0, this->_servers[it->second].get_config().get_client_max_body_size());
					received = recv(this->_fds[i].fd, buffer, sizeof(buffer), MSG_CTRUNC | MSG_DONTWAIT);
					std::cout << "REAL BODY" << std::endl;
					std::cout << buffer << std::endl;
					std::string request_body = buffer;
					if (received > this->_servers[it->second].get_config().get_client_max_body_size())
					{
						std::cout << "Client intended to send too large body." << std::endl;
						close_connection = true;
					}
					else if (received < 0)
					{
						// removed the errno if statement, needs to be tested
						close_connection = true;
						perror("recv");
					}
					else if (received == 0)
					{
						printf("Connection closed\n");
						close_connection = true;
					}
					if (!close_connection)
					{
						/* [ prepare response ] */
						httpHeader request(buffer);
						request.printHeader();
						memset(buffer, 0, this->_servers[it->second].get_config().get_client_max_body_size());
						std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
						response_it->second.new_request(request);
						response_it->second.send_response();
						if (response_it->second.is_cgi())
						{
							CGI cgi(response_it->second, request_body);
							if (response_it->second.getRequest().getMethod() == POST) {
								cgi.fill_in_body();
							}
							int fd = cgi.initPipe();
							if (fd < 0)
								std::cout << RED << "internal server error -> send 500" << RESET << std::endl; //TODO internal server error - 500
							else
							{
								this->_fds[_nfds].fd = fd;
								this->_fds[_nfds].events = POLLIN;
								this->_fds[_nfds].revents = 0;
								this->_fds[i].events = POLLOUT;
								_nfds++;
								cgi.handle_cgi();
								this->_cgis.insert(std::map<int, CGI>::value_type(fd, cgi));
								response_it->second.setCGIFd(fd);
							}
						}
						if (!response_it->second.response_complete())
							this->_fds[i].events = POLLIN | POLLOUT;
					}
					else
					{
						this->_responses.erase(this->_fds[i].fd);
						close(this->_fds[i].fd);
						this->_fds[i].fd = -1;
						compress_array = true;
					}
				}
				else
				{
					// cgi pipe end activity
					char	buffer[1000];
					memset(buffer, 0, 1000);
					std::map<int, CGI>::iterator it = this->_cgis.find(this->_fds[i].fd);
					ssize_t ret = read(this->_fds[i].fd, buffer, sizeof(buffer));
					if (!ret)
					{
						it->second.readComplete();
						// it->second.sendResponse();
						// for (int j = 0; j < _nfds; j++)
						// {
						// 	if (it->second.getResponse().getConnFd() == this->_fds[j].fd)
						// 	{
						// 		this->_fds[j].events = POLLIN;
						// 		break ;
						// 	}
						// }
						// this->_cgis.erase(it);
						// close(this->_fds[i].fd);
						// this->_fds[i].fd = -1;
						// compress_array = true;
					}
					else if (ret > 0)
						it->second.add_to_buffer(buffer);
					else
						perror("read");
				}
			}
			if (this->_fds[i].revents & POLLHUP && this->_cgis.find(this->_fds[i].fd) != this->_cgis.end())
			{
				// cgi pipe remote end closed -> read the remaining and close local end
				char	buffer[1000];
				memset(buffer, 0, 1000);
				while (read(this->_fds[i].fd, buffer, sizeof(buffer)) > 0)
				{
					this->_cgis.find(this->_fds[i].fd)->second.add_to_buffer(buffer);
					memset(buffer, 0, 1000);
				}
				//this->_cgis.find(this->_fds[i].fd)->second.sendResponse();
				//this->_cgis.erase(this->_fds[i].fd);
				this->_cgis.find(this->_fds[i].fd)->second.readComplete();
				close(this->_fds[i].fd);
				this->_fds[i].fd = -1;
				compress_array = true;
			}
			if (this->_fds[i].revents & POLLOUT)
			{
				//std::cout << RED << "POLLOUT EVENT" << RESET << std::endl;
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				if (response_it->second.is_cgi())
				{
					std::map<int, CGI>::iterator cgi_it = this->_cgis.find(response_it->second.getCGIFd());
					if (cgi_it->second.doneReading())
					{
						cgi_it->second.sendResponse();
						this->_cgis.erase(cgi_it);
						this->_fds[i].events = POLLIN;
					}
				}
				else
				{
					response_it->second.send_response();
					if (response_it->second.response_complete())
						this->_fds[i].events = POLLIN;
				}
			}
			nbr_fd_ready--;
			if (nbr_fd_ready == 0)
				break ;
		}
		if (compress_array)
		{
			compress_array = false;
			for (int i = 2; i < this->_nfds; i++)
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
					i--;
					this->_nfds--;
				}
			}
		}
		for (int i = 0; i < this->_nfds; i++)
			this->_fds[i].revents = 0;
    }
	for (int i = 0; i < this->_nfds; i++)
	{
		if (this->_fds[i].fd >= 0)
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
