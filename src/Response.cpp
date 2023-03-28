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

void Response::getPath()
{
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
		return;
	}
    else
    	_respond_path = _request.getUri();
    _respond_path = _config.get_root() + clean_response_path(_respond_path);
}

int 	Response::send_response()
{
	int	sent;
	std::ostringstream response_stream;
	_respond_path.clear();
	_response_body.clear();
	_response.clear();
	_is_cgi = false;
	if (_request.getMethod() > 2)
	{
		response_stream << createError(501);
	}
	else if(!getConfig().find_location("/")->check_method_at(_request.getMethod()))
	{
		response_stream << createError(405);
	}
	else
	{
		getPath();
		if (_is_cgi)
			return 0;
		std::ifstream file(_respond_path.c_str());
		std::cout << RED << _respond_path << RESET << std::endl;
		if (!file.is_open())
		{
			std::cout << std::endl << RED << "CANT OPEN" << RESET << std::endl << std::endl;
			response_stream << createError(404);
		}
		else
		{
			if (_request.getMethod() == GET)
			{
				if (_is_cgi)
					return 0;
				responseToGET(file, _request.getUri(), response_stream);
			}
			if (_request.getMethod() == POST)
			{
				if (_is_cgi)
					return 0;
				// responseToPOST(_request, response_stream);
				// response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A POST REQUEST";
			}
			if (_request.getMethod() == DELETE)
			{
				response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A DELETE REQUEST";
			}

		}
    	file.close();
	}
/* --------------------------------------------------------------------------- */	
	// Send the response to the client
	_response = response_stream.str();
	//std::cout << _response << std::endl;
	sent = send(this->_conn_fd, _response.c_str(), _response.length(), MSG_DONTWAIT);
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
		send_404(this->_config.get_root(), response_stream); //TODO send -> 415 Unsupported media type
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
//	// 		response_stream << HTTPS_OK << "Content-Length: " << _response_body.length() << "\n" << "Connection: Keep-Alive\n" << _types.get_content_type(".html") << handler.get_response_body();
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

std::string	Response::createError(int errorNumber)
{
	std::string			response_body;
	std::string			errorName;
	std::ostringstream	response_stream;
	std::ifstream error(getErrorPath(errorNumber, errorName).c_str());

	if(!error.is_open())
		std::cerr << RED << "error opening " << errorNumber << " file\n" << RESET << std::endl;
	else
	{
		std::stringstream	file_buffer;
		file_buffer << error.rdbuf();
		std::cout << BLUE << file_buffer.str() << RESET << std::endl;
		response_body = file_buffer.str();
		response_stream << "HTTP/1.1 " << errorNumber << " " << errorName << "\r\n" << "Content-Length: " << response_body.length() <<"\r\n\r\n";
		response_stream << response_body;
		error.close();
	}
	return (response_stream.str());
}

std::string Response::getErrorPath(int &errorNumber, std::string& errorName)
{
	std::string			path;
	std::string			errorDir = "docs/www/error/";

	switch (errorNumber)
	{
		case 400:
			path = errorDir + "400_BadRequest.html";
			errorName = "Bad Request";
			break;
		case 401:
			path = errorDir + "401_Unauthorized.html";
			errorName = "Unauthorized";
			break;
		case 403:
			path = errorDir + "403_Forbidden.html";
			errorName = "Forbidden";
			break;
		case 404:
			path = errorDir + "404_NotFound.html";
			errorName = "Not Found";
			break;
		case 405:
			path = errorDir + "405_MethodNotAllowed.html";
			errorName = "Method Not Allowed";
			break;
		case 406:
			path = errorDir + "406_NotAcceptable.html";
			errorName = "Not Acceptable";
			break;
		case 407:
			path = errorDir + "407_ProxyAuthenticationRequired.html";
			errorName = "Proxy Authentication Required";
			break;
		case 408:
			path = errorDir + "408_RequestTimeout.html";
			errorName = "Request Timeout";
			break;
		case 411:
			path = errorDir + "411_LengthRequired.html";
			errorName = "Length Required";
			break;
		case 413:
			path = errorDir + "413_PayloadTooLarge.html";
			errorName = "Payload Length";
			break;
		case 414:
			path = errorDir + "414_URITooLarge.html";
			errorName = "URI Length";
			break;
		case 415:
			path = errorDir + "415_UnsupportedMediaType.html";
			errorName = "Unsupported Media Type";
			break;
		case 500:
			path = errorDir + "500_InternalServer.html";
			errorName = "Internal Server";
			break;
		case 501:
			path = errorDir + "501_NotImplemented.html";
			errorName = "Not Implemented";
			break;
		case 502:
			path = errorDir + "502_BadGateaway.html";
			errorName = "Bad Gateaway";
			break;
		case 503:
			path = errorDir + "503_ServiceUnavailable.html";
			errorName = "Service Unavailable";
			break;
		case 504:
			path = errorDir + "504_GateawayTimeout.html";
			errorName = "Gateaway Timeout";
			break;
		default:
			path = errorDir + "418_Imateapot.html";
			errorName = "I'm a teapot";
			errorNumber = 418;
			break;
	}
	return (path);
}