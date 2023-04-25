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
	this->_fds = NULL;
	this->_listening = true;
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
		if (this->_host_serv.size() > FD_SETSIZE >> 2)
			throw std::logic_error("ERROR: --- Too many servers to handle ---\n");
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

// Initialises the pollfd array with the server socket fds.
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

// Moves active fds to the front of the pollfd array.
void ServerManager::compress_array()
{
	for (int i = this->_n_servers; i < this->_nfds; i++)
	{
		if (this->_fds[i].fd == -1)
		{
			int j = this->_nfds - 1;
			for (; j > i; j--)
			{
				if (this->_fds[j].fd != -1)
				{
					this->_fds[i].fd = this->_fds[j].fd;
					this->_fds[i].events = this->_fds[j].events;
					this->_fds[j].fd = -1;
					this->_fds[j].events = 0;
					this->_nfds--;
					break ;
				}
			}
			if (j == i)
			{
				this->_fds[i].events = 0;
				this->_nfds--;
			}
		}
	}
}

// Main server loop. Uses poll to check for events on fds.
int ServerManager::run_servers()
{
	int	nbr_fd_ready;
	while (SWITCH)
	{
		compress_array();
		if (this->_nfds >= FD_SETSIZE >> 1 && this->_listening)
			stopListening();
		else if (!this->_listening)
			startListening();
		nbr_fd_ready = poll(this->_fds, this->_nfds, -1);
		if (nbr_fd_ready == -1)
			continue;
		for (int i = 0; i < this->_nfds; i++)
		{
			if (nbr_fd_ready == 0)
				break;
			if (this->_fds[i].revents == 0)
				continue;
			else
				nbr_fd_ready--;
			if (this->_fds[i].revents & POLLIN)
			{
				if (i < this->_n_servers)
				{
					acceptConnection(i);
					continue;
				}
<<<<<<< HEAD
				if (fcntl(connection_fd, F_SETFL, O_NONBLOCK) == -1)
				{
					perror("fcntl set_flags");
					close(connection_fd);
					continue;
				}
				std::string address(inet_ntoa(addr.sin_addr));
				// std::cout << "New connection from " << address << " on socket " << connection_fd << "\n";
				// std::cout << "nfds: " << this->_nfds << "\n";
				this->_fds[this->_nfds].fd = connection_fd;
				this->_fds[this->_nfds].events = POLLIN;
				// std::cout << "inserted into fd array" << std::endl;
				this->_responses.insert(std::map<int, Response>::value_type(this->_fds[this->_nfds].fd, Response(this->_fds[this->_nfds].fd, this->_fds[i].fd, this->_fds, this->_nfds, address)));
				// std::cout << "inserted into response map: " << inserted.second << std::endl;
				this->_nfds++;
				// std::cout << "connection accept complete" << std::endl;
			}
			else if (this->_fds[i].revents & POLLIN)
			{
=======
>>>>>>> ad52d4f7679e3c22dce6752771f82b5f4326bf1c
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				if (response_it != this->_responses.end())
				{
					if (!readRequest(response_it->second, i))  // read from client
						continue;
				}
				else
					readCGI(i); // read from cgi
			}
			if (this->_fds[i].revents & POLLHUP)
			{
				std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
				if (cgi_it != this->_cgis.end())
					readRemainingCGI(cgi_it->second, i); // read remaining data from cgi
			}
			else if (this->_fds[i].revents & POLLOUT)
			{
				std::map<int, Response>::iterator response_it = this->_responses.find(this->_fds[i].fd);
				if (response_it != this->_responses.end())
					writeResponse(response_it->second, i); // write to client
				else
					writeCGI(i);
			}
		}
	}
	closeFds();
	return EXIT_SUCCESS;
}

void	ServerManager::close_connection(Response &response, int i)
{
	this->_addr_fd.erase(response.getAddress());
	this->_responses.erase(this->_fds[i].fd);
	if (this->_fds[i].fd > 0)
	{
		close(this->_fds[i].fd);
		this->_fds[i].fd = -1;
	}
}

// Opens pipes and then calls handleCGI() to create a new CGI process.
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
	else if (this->_nfds >= FD_SETSIZE - 10)
	{
		cgi.setErrNo(2);
		cgi.sendResponse();
		close_connection(cgi.getResponse(), i);
		return false;
	}
	else
	{
		std::pair<std::map<int, CGI>::iterator, bool> ret_pair = this->_cgis.insert(std::map<int, CGI>::value_type(out_fd, cgi));
		std::pair<std::map<int, int>::iterator, bool> cgi_ret_pair = this->_cgi_fds.insert(std::map<int, int>::value_type(in_fd, out_fd));
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
			cgi_it->second.closePipes();
			cgi_it->second.sendResponse();
			//cgi_it->second.getResponse().getRequest().printHeader();
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
	return (0);
}

// extracts port from host
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

// when server is too busy, stops listening for new connections
void ServerManager::stopListening()
{
	for (int i = 0; i < this->_n_servers; i++)
		this->_fds[i].events = 0;
	this->_listening = false;
}

void ServerManager::startListening()
{
	for (int i = 0; i < this->_n_servers; i++)
		this->_fds[i].events = POLLIN;
	this->_listening = true;
}

/**
 * @brief Accepts a connection and adds it to the pollfd array
 */
void ServerManager::acceptConnection(int i)
{
	struct sockaddr_in addr;
	socklen_t addr_len = sizeof(sockaddr_in);
	int connection_fd = accept(this->_fds[i].fd, (struct sockaddr *)&addr, &addr_len);
	if (connection_fd < 0)
		return;
	std::string address(inet_ntoa(addr.sin_addr));
	this->_fds[this->_nfds].fd = connection_fd;
	this->_fds[this->_nfds].events = POLLIN;
	this->_responses.insert(std::map<int, Response>::value_type(this->_fds[this->_nfds].fd, Response(this->_fds[this->_nfds].fd, this->_fds[i].fd, this->_fds, this->_nfds, address)));
	this->_nfds++;
}

// reads from socket and stores in buffer
bool	ServerManager::readRequest(Response &response, int i)
{
	char	buffer[BUFFER_SIZE];
	ssize_t		received;
	memset(buffer, 0, sizeof(buffer));
	received = recv(this->_fds[i].fd, buffer, sizeof(buffer), MSG_DONTWAIT);
	if (received < 0)
		return false;
	else if (received == 0)
	{
		this->close_connection(response, i);
		return false;
	}
	else
	{
		// std::cout << "buffer:\n" << buffer << "end of buffer." << std::endl;
		/* [ prepare response ] */
		std::map<int, CGI>::iterator cgi_it = this->_cgis.find(response.getCGIFd());
		if (cgi_it != this->_cgis.end() && !cgi_it->second.completeContent()) // cgi fd
		{
			if (cgi_it->second.getResponse().isChunked())
				cgi_it->second.mergeChunk(buffer, received);
			else
				cgi_it->second.storeBuffer(buffer, received);
		}
		else //no ongoing cgi
		{
			httpHeader request(buffer);
			std::string host(request.get_single_header("host"));
			std::map<std::string, Server>::iterator serv_it = this->_host_serv.find(host);
			if (serv_it != this->_host_serv.end())
				response.newConfig(serv_it->second.get_config());
			else
			{
				std::map<std::string, std::string>::iterator def_it = this->_default_host.find(host);
				if (def_it != this->_default_host.end())
				{
					// std::cout << BLUE << "Using default server for IP/Port: " << def_it->second << "\n" << RESET;
					response.newConfig(this->_host_serv.find(def_it->second)->second.get_config());
				}
				else
				{
					// std::cout << BLUE << "Finding default server for port\n" << RESET;
					host = getDefPort(host);
					if (host.empty())
					{
						// std::cout << RED << "No default server found for port. Request from: " << request.get_single_header("host") << "\n" << RESET;
						if (static_cast<int>(request.isError()) == 2)
							send(this->_fds[i].fd, "HTTP/1.1 Not Found\r\n\r\n", 26, 0);
						else if (static_cast<int>(request.isError()) == 1)
							send(this->_fds[i].fd, "HTTP/1.1 414 Bad Request\r\n\r\n", 28, 0);
						close_connection(response, i);
						return false;
					}
					// std::cout << BLUE << "Using default server for port: " << host << "\n" << RESET;
					response.newConfig(this->_host_serv.find(host)->second.get_config());
				}
			}
			response.new_request(request);
			response.handle_response();
			response.getRequest().setStatusCode(get_cgi_response(response.get_response()));
			// if (response.is_cgi() == false)
			// {
			// 	response.getRequest().printHeader();
			// }
			if (response.shouldClose())
				close_connection(response, i);
			else if (response.is_cgi()) // initialise cgi process
			{
				if (this->initCGI(response, buffer, received, i, request))
					this->_fds[i].events = POLLIN | POLLOUT;
			}
		}
	}
	return true;
}

// read from cgi out
void ServerManager::readCGI(int i)
{
	char	buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	std::map<int, CGI>::iterator cgi_it = this->_cgis.find(this->_fds[i].fd);
	if (cgi_it == this->_cgis.end())
		return;
	ssize_t rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
	if (rec > 0)
		cgi_it->second.add_to_buffer(buffer, rec);
	else if (rec < 0)
		perror("read");
	return;
}

// read remaining data from cgi out
void	ServerManager::readRemainingCGI(CGI &cgi, int i)
{
	char	buffer[BUFFER_SIZE];
	memset(buffer, 0, BUFFER_SIZE);
	ssize_t rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
	while (rec > 0)
	{
		cgi.add_to_buffer(buffer, rec);
		rec = read(this->_fds[i].fd, buffer, sizeof(buffer));
	}
	if (rec == 0)
	{
		cgi.setReadComplete();
		this->_fds[i].fd = -1;
	}
	else
		std::cout << RED << "Segmentation Fault\n" << RESET;
}

// send to client socket
void	ServerManager::writeResponse(Response &response, int i)
{
	if (!response.isComplete())
	{
		response.send_response();
		if (response.isComplete())
			this->_fds[i].events = POLLIN;
	}
	else
	{
		std::map<int, CGI>::iterator cgi_it = this->_cgis.find(response.getCGIFd());
		if (cgi_it != this->_cgis.end() && cgi_it->second.readComplete()) // if done reading from cgi out, send response to client
		{
			if (cgi_it->second.sendResponse())
			{
				//cgi_it->second.getResponse().getRequest().printHeader();
				close(response.getCGIFd());
				response.setCGIFd(-1);
				this->_cgis.erase(cgi_it);
				this->_fds[i].events = POLLIN;
			}
		}
	}
}

// write request to cgi in
void ServerManager::writeCGI(int i)
{
	std::map<int, int>::iterator cgi_fd_it = this->_cgi_fds.find(this->_fds[i].fd);
	if (cgi_fd_it != this->_cgi_fds.end())
	{
		std::map<int, CGI>::iterator cgi_it = this->_cgis.find(cgi_fd_it->second);
		if (!cgi_it->second.bodySentCGI())
		{
			if (!cgi_it->second.getResponse().isChunked())
				cgi_it->second.writeToCGI();
		}
		else if (cgi_it->second.bodySentCGI())
		{
			close(this->_fds[i].fd);
			this->_fds[i].fd = -1;
			this->_cgi_fds.erase(cgi_fd_it);
		}
	}
}

void ServerManager::closeFds()
{
	for (int i = 0; i < this->_nfds; i++)
	{
		if (this->_fds[i].fd > 0)
		{
			close(this->_fds[i].fd);
			this->_fds[i].fd = -1;
		}
	}
}
