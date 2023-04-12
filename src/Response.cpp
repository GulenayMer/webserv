#include "../include/Response.hpp"

Response::Response(int conn_fd, int server_fd, Config& config, struct pollfd* fds, int nfds, std::string addr): _config(config)
{
    _conn_fd = conn_fd;
    _server_fd = server_fd;
	_bytes_sent = 0;
	_received_bytes = 0;
	_fds = fds;
	_nfds = nfds;
	_cgi_fd = -1;
	_error = false;
	_is_complete = true;
	_to_close = false;
	_addr = addr;
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
		_cgi_fd = src._cgi_fd;
		_bytes_sent = src._bytes_sent;
		_received_bytes = src._received_bytes;
		_is_cgi = src._is_cgi;
		_is_chunked = src._is_chunked;
		_types = src._types;
		_response_body = src._response_body;
		_respond_path = src._respond_path;
		_response = src._response;
		_config = src._config;
		_request = src._request;
		_error = src._error;
		_is_complete = src._is_complete;
		_to_close = src._to_close;
		_addr = src._addr;
		_ext = src._ext;
	}
	return *this;
}

Response::~Response() {}

void Response::getPath()
{
	_respond_path.clear();
	_response_body.clear();
	_response.clear();
	_is_cgi = false;
	// std::cout << "INDEX: " << _config.get_index() << std::endl;
	// if (_request.getUri() == "/")
	// 	_respond_path = _config.get_index();
	if (this->checkCGI())
	{
		_is_cgi = true;
		return;
	}
    else
		_respond_path = _request->getUri();
	// std::cout << "GETPATH RESPOND PATH: " << this->_request.getUri() << std::endl;
    //_respond_path = _config.get_root() + clean_response_path(_respond_path);
}

int 	Response::handle_response()
{
	std::ostringstream response_stream;
	_respond_path.clear();
	_response_body.clear();
	_response.clear();
	setChunked();
	_is_cgi = false;
	if (!handle_response_error(response_stream))
	{
		getPath();
		size_t pos = _request->getUri().find_last_of(".");
		if (!_is_cgi && _is_chunked)
		{
			response_stream << createError(404, &this->getConfig());
			_to_close = true;
		}
		else if (!_is_cgi && !_is_dir && pos != std::string::npos && _types.get_content_type(&this->_request->getUri()[pos]).empty())
		{
			response_stream << createError(500, &this->getConfig());
			_to_close = true;
		}
		else if(this->_is_dir && _request->getMethod() == GET)
		{
			if (this->_location.get_autoindex() && this->_list_dir)
				response_stream << directoryListing(_respond_path);
			else
				response_stream << createError(404, &this->getConfig());
		}
		else if (_is_cgi)
		{
			pos = this->_request->getUri().find_last_of("/");
			if (pos != std::string::npos && !dir_exists(this->_request->getUri().substr(0, pos)))
			{
				response_stream << createError(404, &this->getConfig());
				this->_is_cgi = false;
			}
			else if (!this->_location.allow_cgi())
			{
				response_stream << createError(403, &this->getConfig());
				this->_is_cgi = false;
			}
			else
				return 0;
		}
		else
		{
			std::ifstream file(_respond_path.c_str());
			// std::cout << RED << _respond_path << RESET << std::endl;
			if (!file.is_open())
			{
				// std::cout << std::endl << RED << "Unable to open requested file." << RESET << std::endl << std::endl;
				response_stream << createError(404, &this->getConfig());
				_to_close = true;
			}
			else
			{
				if (_request->getMethod() == GET)
				{
					if (_is_cgi)
						return 0;
					responseToGET(file, _request->getUri(), response_stream);
				}
				if (_request->getMethod() == POST)
				{
					if (_is_cgi)
						return 0;
					// responseToPOST(_request, response_stream);
					// response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A POST REQUEST";
				}
				if (_request->getMethod() == DELETE)
				{
					responseToDELETE(response_stream);
					// response_stream << HTTPS_OK << _types.get_content_type(".html") << "THERE WAS A DELETE REQUEST";
				}
			}
			file.close();
		}
	}
	return (send_response(response_stream));
}

int 	Response::handle_response_error(std::ostringstream& response_stream)
{
	if (_request->isError()) {
		response_stream << createError(414, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (!_request->isHttp11()) {
		response_stream << createError(505, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (_request->getContentLength() > _config.get_client_max_body_size()) {
		std::cout << "content length too large" << std::endl;
		response_stream << createError(413, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (_request->getMethod() > 2) {
		response_stream << createError(501, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if(!_location.check_method_at(_request->getMethod())) {
		response_stream << createError(405, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (_request->getMethod() == POST && this->_request->get_single_header("Content-Length").empty() && !this->_is_chunked) {
		response_stream << createError(411, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (_request->getMethod() == POST && _request->getContentLength() == 0 && !this->_is_chunked) {
		response_stream << createError(400, &this->getConfig());
		_to_close = true;
		return 1;
	}
	else if (_request->getMethod() == POST && _request->getContentLength() > _config.get_client_max_body_size()) {
		response_stream << createError(413, &this->getConfig());
		_to_close = true;
		return 1;
	}
	return 0;
}


int 	Response::send_response(std::ostringstream& response_stream)
{
	_response = response_stream.str();
	int	sent = send(this->_conn_fd, _response.c_str(), _response.length(), MSG_DONTWAIT);
	_request->setSentSize(sent);
	if (sent > 0)
	{
		_bytes_sent += sent;
		if (_bytes_sent == _response.length())
		{
			_response.clear();
			_bytes_sent = 0;
		}
	}
	return _error;
}

void	Response::responseToGET(std::ifstream &file, const std::string& path, std::ostringstream &response_stream)
{
	(void)path;
	// std::cout << std::endl << RED << path << RESET << std::endl;
	std::stringstream	file_buffer;
	std::string	type;

	size_t pos;

	pos = _respond_path.find_last_of(".");
	if (pos != std::string::npos)
	{
		type = _types.get_content_type(&_respond_path[pos]);
		if (type.empty())
		{
			std::cout << RED << "Unsupported media type" << RESET << std::endl;
			response_stream << createError(415, &this->getConfig());
			return ;
		}
	}
	else
	{
		type = "Content-Type: text/plain\r\n\r\n";
	}
	file_buffer << file.rdbuf();
	_response_body = file_buffer.str();
	_request->setStatusCode(200);
	response_stream << HTTP_OK << "Content-Length: " << _response_body.length() << "\nConnection: Keep-Alive\n";
	response_stream << type << _response_body;
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
	_request->setStatusCode(404);
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

bool	Response::new_request(httpHeader *request)
{
	this->_request = request;
	this->_to_close = false;
	this->_is_complete = false;
	this->_is_dir = false;
	this->_list_dir = false;
	this->_received_bytes = 0;
	std::map<std::string, Location>::iterator loc_it;
	std::string uri = request->getUri();
	size_t pos;
	if (uri.find_first_of(".") == std::string::npos)
	{
		this->_is_dir = true;
		if (uri.length() > 1 && uri[uri.length() - 1] == '/')
		{
			uri.erase(uri.length() - 1);
			this->_list_dir = true;
		}
	}
	size_t size = uri.size();
	pos = uri.length() - 1;
	// std::cout << "new request: " << uri << std::endl;
	while (!uri.empty())
	{
		if (uri.length() > 1 && uri[uri.length() -1] == '/')
			uri.erase(uri.length() - 1);
		// std::cout << uri << std::endl;
		std::map<std::string, std::string>::iterator red_it = this->getConfig().getRedirection().find(uri);
		if (red_it != this->getConfig().getRedirection().end())
		{
			size += red_it->second.size() - uri.size();
			// std::cout << "SIZE: " << size << std::endl;
			uri = red_it->second;
			loc_it = this->getConfig().get_location().find(red_it->second);
		}
		else
			loc_it = this->getConfig().get_location().find(uri);
		if (loc_it != this->getConfig().get_location().end())
		{
			this->_location = loc_it->second;
			// std::cout << "OLD URI: " << this->_request.getUri() << std::endl;
			if (this->_is_dir)
			{
				if (!this->_location.get_index().empty() && uri.size() == size)
				{
					this->_request->setURI(this->_location.get_root() + this->_location.get_index());
					this->_is_dir = false;
				}
				else
					this->_request->setURI(this->_location.get_root() + &request->getUri()[pos + 1]);

			}
			else
				this->_request->setURI(this->_location.get_root() + &request->getUri()[pos + 1]);
			// std::cout << "NEW URI: " << this->_request.getUri() << std::endl;
			return true;
		}
		if (pos > 0)
			pos = uri.find_last_of("/", pos - 1);
		if (pos == std::string::npos)
			break;
		uri.erase(pos + 1);
	}
	return false;
}

bool	Response::response_complete() const
{
	if (_response.empty())
		return true;
	return false;
}

void	Response::responseToDELETE(std::ostringstream &response_stream)
{
	/* 
		DELETE REQUEST
		1. Get path to the requested resource path
		2. Check if the resource exists
		3. delete resource
		4. send 204 status
		or/else
		5. send 404
	*/
	// std::cout << RED << "PATH : " << _respond_path << RESET << std::endl;
	std::ifstream	pathTest(_respond_path.c_str());
	if (!pathTest.is_open())
	{
		// std::cout << RED << "this 404" << RESET << std::endl;
		send_404(this->_config.get_root(), response_stream);
	}
	else
	{
		// delete file
		if (remove(_respond_path.c_str()) == -1)
		{
			// std::cout << RED << "next 404" << RESET << std::endl;
			send_404(this->_config.get_root(), response_stream);
		}
		else
		{
			response_stream << HTTP_204 << _types.get_content_type(".txt");
			_request->setStatusCode(204);
		}
	}
	pathTest.close();
}

bool	Response::is_cgi()
{
	return _is_cgi;
}

Config &Response::getConfig()
{
	return this->_config;
}

httpHeader *Response::getRequest()
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

/**
 * @brief 
 * 
 * @param errorNumber Standard error code
 * @param config Needs a config to access paths, should be NULL if error is pre-config creation
 * @return std::string 
 */
std::string	Response::createError(int errorNumber, Config* config)
{
	std::string			response_body;
	std::string			errorName;
	std::ostringstream	response_stream;
	std::string			error_body = getErrorPath(errorNumber, errorName, config);
	std::ifstream error(error_body.c_str());

	_request->setStatusCode(errorNumber);
	if(!error.is_open())
		std::cerr << RED << "error opening " << errorNumber << " file at " << error_body << RESET << std::endl;
	else
	{
		std::stringstream	file_buffer;
		file_buffer << error.rdbuf();
		response_body = file_buffer.str();
		response_stream << "HTTP/1.1 " << errorNumber << " " << errorName << "\r\n" << "Content-Type: text/html; charset=utf-8\r\n" << "Content-Length: " << response_body.length() << "\r\n\r\n";
		response_stream << response_body;
		error.close();
	}
	return (response_stream.str());
}

std::string Response::getErrorPath(int &errorNumber, std::string& errorName, Config* config)
{
	std::string			error_path;

	
	switch (errorNumber)
	{
		case 400:
			errorName = "Bad Request";
			break;
		case 401:
			errorName = "Unauthorized";
			break;
		case 403:
			errorName = "Forbidden";
			break;
		case 404:
			errorName = "Not Found";
			break;
		case 405:
			errorName = "Method Not Allowed";
			break;
		case 406:
			errorName = "Not Acceptable";
			break;
		case 407:
			errorName = "Proxy Authentication Required";
			break;
		case 408:
			errorName = "Request Timeout";
			break;
		case 411:
			errorName = "Length Required";
			break;
		case 413:
			errorName = "Payload Length";
			break;
		case 414:
			errorName = "URI Length";
			break;
		case 415:
			errorName = "Unsupported Media Type";
			break;
		case 429:
			errorName = "Many Requests";
			break;
		case 500:
			errorName = "Internal Server Error";
			break;
		case 501:
			errorName = "Not Implemented";
			break;
		case 502:
			errorName = "Bad Gateaway";
			break;
		case 503:
			errorName = "Service Unavailable";
			break;
		case 504:
			errorName = "Gateaway Timeout";
			break;
		case 505:
			errorName = "Unsupported HTTP Version";
			break;
		default:
			errorName = "I'm a teapot";
			errorNumber = 418;
			break;
	}
	error_path = config->get_error_path(errorNumber);
	return (error_path);
}

bool Response::checkCGI()
{
	size_t pos = this->_request->getUri().find_last_of(".");
	size_t pos2 = this->_request->getUri().find_last_of("/");
	if (this->getConfig().getIntrPath().empty())
		return false;
	if (pos != std::string::npos)
	{
		if (pos2 != std::string::npos && pos < pos2)
		{
			this->_is_dir = true;
			return false;
		}
		this->_ext = this->_request->getUri().substr(pos);
		std::map<std::string, std::string>::iterator it = this->getConfig().getIntrPath().find(this->_ext);
		if (it != this->getConfig().getIntrPath().end())
			return true;
	}
	this->_ext.clear();
	return false;
}

bool	Response::checkPermissions()
{
	std::string path = this->getRequest()->getUri();
	if (path.size() == 1)
		path = this->_location.get_root() + this->_location.get_index();
	else
		path = this->_location.get_root() + &path[1];
	return dir_exists(path);
}

void	Response::setCompletion(bool complete)
{
	this->_is_complete = complete;
}

bool	Response::isComplete()
{
	return this->_is_complete;
}

std::string &Response::getAddress()
{
	return this->_addr;
}

bool Response::shouldClose()
{
	return this->_to_close;
}


/* 
    directoryExists()
        checks if a directory exists at the specified path
        using the stat() system call.
        If the path exists and is a directory,
        return true; otherwise, return false.
*/
bool Response::directoryExists(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

/* 
    directoryListing()
        returns a string containing an HTML directory listing
        of the specified directory.
*/
std::string Response::directoryListing(std::string uri)
{
    DIR *dir;
    struct dirent *ent;

	if (!directoryExists(uri.c_str()))
		return (Response::createError(404, &this->getConfig()));
	
	std::ostringstream outfile;

    outfile << "<!DOCTYPE html>\n";
    outfile << "<html>\n";
    outfile << "<head>\n";
    outfile << "<title>Directory Listing</title>\n";
    outfile << "</head>\n";
    outfile << "<body>\n";
    outfile << "<h1>Directory Listing</h1>\n";
    outfile << "<ul>\n";

    if ((dir = opendir(uri.c_str())) != NULL)
	{
        while ((ent = readdir(dir)) != NULL)
		{
			if (dir_exists(uri + ent->d_name))
				outfile << "<li><a href=\"" << ent->d_name <<"/\" >" << ent->d_name << "</a></li>" << std::endl;
			else
				outfile << "<li><a href=\"" << ent->d_name <<"\" >" << ent->d_name << "</a></li>" << std::endl;
        }
        closedir(dir);
    }

    outfile << "</ul>\n";
    outfile << "</body>\n";
    outfile << "</html>\n";
	std::string body(outfile.str());
	std::ostringstream message; 
	message << HTTP_OK << "Content-Length: " << body.length() << "\n" << _types.get_content_type(".html") << "\r\n\r\n" << body;
	return (message.str());
}


bool Response::dir_exists(const std::string& dirName_in)
{
	int ret = 0;
	struct stat info;

	ret = stat(dirName_in.c_str(), &info);
	if( ret != 0 )
	{
		return false;  // something went wrong
	}
	else if( info.st_mode & S_IFDIR )
	{
		return true;   // this is a directory
	}
	else
	{
		return false;  // this is not a directory
	}
}

ssize_t Response::receivedBytes(ssize_t received)
{
	this->_received_bytes += received;
	// std::cout << "total received: " << _received_bytes << " content: " << this->getRequest().getContentLength() << std::endl;
	return (this->getRequest()->getContentLength() - this->_received_bytes);
}

// const char* dir_path = "/path/to/directory";

//     // check if the directory exists
//     struct stat st;
//     if (stat(dir_path, &st) == 0 && S_ISDIR(st.st_mode)) {
//         std::cout << dir_path << " is a directory" << std::endl;
//     } else {
//         std::cout << dir_path << " is not a directory" << std::endl;
//     }

std::string &Response::getExt()
{
	return this->_ext;
}

void	Response::setChunked()
{
	if (this->_request->get_single_header("Transfer-Encoding") == "chunked")
		this->_is_chunked = true;
	else if (this->_request->get_single_header("transfer-encoding") == "chunked")
		this->_is_chunked = true;
	else
		this->_is_chunked = false;
}

void	Response::finishChunk()
{
	this->_is_chunked = false;
}

bool	Response::isChunked()
{
	return this->_is_chunked;
}
