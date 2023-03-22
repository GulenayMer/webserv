#include "../include/Response.hpp"

Response::Response(int conn_fd, int server_fd, Config& config, struct pollfd* fds, int nfds): _config(config)
{
    _conn_fd = conn_fd;
    _server_fd = server_fd;
	_bytes_sent = 0;
	_fds = fds;
	_nfds = nfds;
}

Response::Response(const Response &src)
{
	*this = src;
}

Response &Response::operator=(const Response &src)
{
	if (this != &src)
	{
		_httpVersion = src._httpVersion;
		_response_number = src._response_number;
        _conn_fd = src._conn_fd;
        _server_fd = src._server_fd;
		_bytes_sent = src._bytes_sent;
		_req_uri = src._req_uri;
		_is_cgi = src._is_cgi;
        _types = src._types;
		_response_body = src._response_body;
		_respond_path = src._respond_path;
		_response = src._response;
		_config = src._config;
		_request = src._request;
	}
	return *this;
}

Response::~Response()
{

}

int 	Response::send_response()
{
	int	sent;
	std::ostringstream response_stream;
/* --------------------------------------------------------------------------- */
    //TODO get path function
	_respond_path.clear();
	_response_body.clear();
	_response.clear();
	_is_cgi = false;
	if (_request.getUri() == "/")
		_respond_path = _config.get_index();
	else if (_request.getUri().find("cgi-bin") != std::string::npos)
	{
		_is_cgi = true;
		//this->_cgi.setVars(_config, _request);
		return 0;
	}
    else
    	_respond_path = _request.getUri();
    _respond_path = _config.get_root() + clean_response_path(_respond_path);
    std::ifstream file(_respond_path.c_str());
/* --------------------------------------------------------------------------- */
    std::cout << RED << _respond_path << RESET << std::endl;
	if (!file.is_open())
	{
		std::cout << std::endl << RED << "CANT OPEN" << RESET << std::endl << std::endl;
    	send_404(_config.get_root(), response_stream);
	}
    else
    {
        /* 
			TODO
				- which response
				- resopose number
				- response type
				- response body
				- send the response
				- close the file
		*/
		if (_request.getMethod() == GET)
		{
			responseToGET(file, _request.getUri(), response_stream);
		}
		// if (_request.getMethod() == POST)
		// {
		// 	responseToPOST(_request, response_stream);
		// 	//response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A POST REQUEST";
		// }
		if (_request.getMethod() == DELETE)
		{
			response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A DELETE REQUEST";
		}

    }
	
	// Send the response to the client
	_response = response_stream.str();
	//std::cout << _response << std::endl;
	sent = send(this->_conn_fd, _response.c_str(), _response.length(), MSG_DONTWAIT);
    file.close();
	if (sent > 0)
	{
		_bytes_sent += sent;
		if (_bytes_sent == _response.length())
		{
			_response.clear();
			_bytes_sent = 0;
		}
	}
	return sent; 
}

void	Response::responseToGET(std::ifstream &file, const std::string& path, std::ostringstream &response_stream)
{
	std::cout << std::endl << RED << path << RESET << std::endl;
	std::stringstream	file_buffer;
	std::string	type;

	type = _types.get_content_type(&_respond_path[_respond_path.find_last_of(".")]);
	if (type.empty())
	{
		send_404(this->_config.get_root(), response_stream);
		return ;
	}
	file_buffer << file.rdbuf();
	_response_body = file_buffer.str();
	response_stream << HTTPS_OK << "Content-Length: " << _response_body.length() << "\nConnection: Keep-Alive\n";
	response_stream << type << _response_body;
	//std::cout << BLUE << type << RESET << std::endl;
	// if (_respond_path.compare(_respond_path.length() - 5, 5, ".html") == 0) {
	// 	std::cout << BLUE <<  "----HTML----" << RESET << std::endl;
	// 	response_stream << _types.get_content_type(".html") << _response_body;				//TODO I don't think we need this anymore
	// }
	// else if (_respond_path.compare(_respond_path.length() - 4, 4, ".ico") == 0)
	// {
	// 	std::cout << BLUE <<  "----ICO----" << RESET << std::endl;
	// 	response_stream << _types.get_content_type(".ico") << _response_body;
	// }
	// else if (_respond_path.compare(_respond_path.length() - 4, 4, ".css") == 0) {
	// 	std::cout << BLUE <<  "----CSS----" << RESET << std::endl;
	// 	response_stream << _types.get_content_type(".css") << _response_body;
	// }
	// else if (_respond_path.compare(_respond_path.length() - 4, 4, ".png") == 0) {
	// 	std::cout << BLUE <<  "----PNG----" << RESET << std::endl;
	// 	response_stream << _types.get_content_type(".png") << _response_body;
	// }
	// else
	// {
	// 	response_stream.str("");
	// 	response_stream.clear();
	// 	send_404(this->_config.get_root(), response_stream);
	// }
}

// void	Response::responseToPOST(const httpHeader request, std::ostringstream &response_stream)
// {
// 	// TODO change CGI constructor to accept httpheader instead of only URI
// 	// CGI handler(this->_config, request, _response_body, _fds, _nfds);
// 	// 	if (handler.handle_cgi() == EXIT_SUCCESS)
// 	// 		response_stream << HTTPS_OK << "Content-Length: " << _response_body.length() << "\n" << "Connection: Keep-Alive\n" << _types.get_content_type(".html") << handler.get_response_body();
// 	// 	else
// 	// 		send_404(this->_config.get_root(), response_stream);
// }

void 	Response::send_404(std::string root, std::ostringstream &response_stream)
{
	std::string response_body;

	// if the file cannot be opened, send a 404 error
    std::ifstream error404((root + this->_config.get_error_path(404)).c_str());
    if (!error404.is_open())
        std::cerr << RED << _404_ERROR << RESET << std::endl;
	else
	{
		std::stringstream	file_buffer;
		file_buffer << error404.rdbuf();
		response_body = file_buffer.str();
		response_stream << "HTTP/1.1 404 Not Found\r\n\r\n";
		response_stream << response_body;
		error404.close();
	}
}

void	Response::new_request(httpHeader &request)
{
	this->_request = request;
}

bool	Response::response_complete() const
{
	if (_response.empty())
		return true;
	return false;
}

bool	Response::is_cgi()
{
	return _is_cgi;
}

Config &Response::getConfig()
{
	return this->_config;
}

httpHeader &Response::getRequest()
{
	return this->_request;
}

int	Response::getConnFd()
{
	return this->_conn_fd;
}

MIME	&Response::getTypes()
{
	return this->_types;
}

void	Response::setCGIFd(int fd)
{
	this->_cgi_fd = fd;
}

int	Response::getCGIFd()
{
	return this->_cgi_fd;
}
