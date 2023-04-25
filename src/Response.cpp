#include "../include/Response.hpp"

/*
	This is where the responses are handled according to requested method
*/

Response::Response(int conn_fd, int server_fd, struct pollfd* fds, int nfds, std::string addr)
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
	_status_code = 0;
	_is_chunked = false;
	_is_cgi = false;
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
		_status_code = src._status_code;
	}
	return *this;
}

Response::~Response() {}

void Response::newConfig(Config &config)
{
	this->_config = config;
}

void Response::getPath()
{
	/* 
		1. which server block?
		2. check location blocks to see if there is a match
			- if there is a match, check if the location block has a root
			- if it does, set the root to the location block root
			- if it doesn't, set the root to the server root
			- if there is no match, set the root to the server root
 	*/

	if (this->_request.getMethod() != DELETE && this->checkCGI())
	{
		_is_cgi = true;
		return;
	}
    else
	{
		std::string	tmp_path(_request.getUri());
		_respond_path = tmp_path;
		while (tmp_path.length() > 1 && tmp_path[tmp_path.length() - 1] == '/')
			tmp_path.erase(tmp_path.length() - 1);
		size_t pos = tmp_path.find_last_of("/");
		if (pos == std::string::npos)
			pos = 0;
		if (tmp_path.find_first_of(".", pos) == std::string::npos)
		{
			std::string dir_name(tmp_path.substr(pos, tmp_path.size() - pos));
			std::map<std::string, Location>::iterator location_it = _config.get_location().find(dir_name);
			if (location_it == _config.get_location().end())
			{
				_is_dir = false;
				_list_dir = false;
				std::string file(_respond_path + dir_name + ".html");
				if (access(file.c_str(), F_OK) != -1)
					_respond_path = file;
				else {
					file = _respond_path + "/index.html";
					if (access(file.c_str(), F_OK) != -1)
						_respond_path = file;
					else
						_is_dir = true;
				}
			}
			else if (location_it->second.get_index().empty())
			{
				_is_dir = false;
				_list_dir = false;
				_respond_path = location_it->second.get_root();
				if (dir_name[0] == '/')
					dir_name = dir_name.substr(1, dir_name.length() - 1);
				std::string file(_respond_path + dir_name + ".html");
				if (access(file.c_str(), F_OK) != -1)
				{
					_respond_path = _respond_path + dir_name + ".html";
					location_it->second.set_index(dir_name);
				}
				else
				{
					file = _respond_path + "/index.html";
					if (access(file.c_str(), F_OK) != -1)
					{
						_respond_path = _respond_path + "index.html";
						location_it->second.set_index("index.html");
					}
					else
						_is_dir = true;
				}
			}
		}
	}
}

void 	Response::handle_response()
{
	std::ostringstream response_stream;
	if (_is_redirect)
	{
		response_stream << redirect(_request.getUri());
		_to_close = true;
	}
	else if (!handle_response_error(response_stream))
	{
		getPath();
		if (!_is_cgi)
			_request.setURI(_respond_path);
		size_t ext_pos = _request.getUri().find_last_of(".");
		if (!_is_cgi && _is_chunked)
		{
			response_stream << createError(404);
			_to_close = true;
		}
		else if (_is_dir && !this->_list_dir)
		{
			response_stream << createError(404);
			_to_close = true;
		}
		else if (!_is_cgi && !_is_dir && ext_pos != std::string::npos && _types.get_content_type(&this->_request.getUri()[ext_pos]).empty())
		{
			response_stream << createError(415);
			_to_close = true;
		}
		else if (this->_list_dir && _request.getMethod() == GET)
		{
			if (this->_location.get_autoindex())
				response_stream << directoryListing(_respond_path);
			else
				response_stream << createError(404);
		}
		else if (_is_cgi)
		{
			size_t pos = this->_request.getUri().find_last_of("/");
			if (pos != std::string::npos && !dir_exists(this->_request.getUri().substr(0, pos)))
			{
				response_stream << createError(404);
				this->_is_cgi = false;
			}
			else if (access(this->_request.getUri().c_str(), F_OK) == -1)
			{
				response_stream << createError(404);
				this->_is_cgi = false;
			}
			else if (!this->_location.allow_cgi())
			{
				response_stream << createError(403);
				this->_is_cgi = false;
			}
			else
			{
				this->_is_complete = true;
				return;
			}
		}
		else
		{
			std::ifstream file(_respond_path.c_str());
			if (access(_respond_path.c_str(), F_OK) == -1)
			{
				response_stream << createError(404);
				_to_close = true;
			}
			else if (access(_respond_path.c_str(), R_OK) == -1)
			{
				response_stream << createError(403);
				_to_close = true;
			}
			else
			{
				if (_request.getMethod() == GET)
					responseToGET(file, ext_pos, response_stream);
				else if (_request.getMethod() == DELETE)
					responseToDELETE(response_stream);
			}
			file.close();
		}
	}
	this->_response = response_stream.str();
	return;
}

int 	Response::handle_response_error(std::ostringstream& response_stream)
{
	if (_request.isError()) {
		uint8_t myByte = _request.isError();
		if (static_cast<int>(myByte) == 2)
			response_stream << createError(400);
		else if (static_cast<int>(myByte) == 1)
			response_stream << createError(414);
		_to_close = true;
		return 1;
	}
	else if (!_request.isHttp11()) {
		response_stream << createError(505);
		_to_close = true;
		return 1;
	}
	else if (_request.getContentLength() > _config.get_client_max_body_size()) {
		response_stream << createError(413);
		_to_close = true;
		return 1;
	}
	else if (_request.getMethod() > 2) {
		response_stream << createError(501);
		_to_close = true;
		return 1;
	}
	else if(!_location.check_method_at(_request.getMethod())) {
		response_stream << createError(405);
		_to_close = true;
		return 1;
	}
	else if (_request.getMethod() == POST && this->_request.get_single_header("content-length").empty() && !this->_is_chunked) {
		response_stream << createError(411);
		_to_close = true;
		return 1;
	}
	else if (_request.getMethod() == POST && _request.getContentLength() == 0 && !this->_is_chunked) {
		response_stream << createError(400);
		_to_close = true;
		return 1;
	}
	else if (_request.getMethod() == POST && _request.getContentLength() > _config.get_client_max_body_size()) {
		response_stream << createError(413);
		_to_close = true;
		return 1;
	}
	return 0;
}

int 	Response::send_response()
{
	int	sent = send(this->_conn_fd, _response.c_str(), _response.length(), MSG_DONTWAIT);
	_request.setSentSize(sent);
	if (sent > 0)
	{
		_bytes_sent += sent;
		if (_bytes_sent == _response.length())
		{
			this->_is_complete = true;
			_bytes_sent = 0;
		}
	}
	return _error;
}

void	Response::responseToGET(std::ifstream &file, size_t &pos, std::ostringstream &response_stream)
{
	std::stringstream	file_buffer;
	std::string	type;

	type = _types.get_content_type(&_respond_path[pos]);
	file_buffer << file.rdbuf();
	_response_body = file_buffer.str();
	response_stream << HTTP_OK << "Content-Length: " << _response_body.length() << "\nConnection: Keep-Alive\n";
	response_stream << type << _response_body;
	this->_status_code = 200;
}

bool	Response::new_request(httpHeader &request)
{
	this->_request = request;
	this->_to_close = false;
	this->_is_complete = false;
	this->_is_dir = false;
	this->_is_redirect = false;
	this->_list_dir = false;
	this->_is_cgi = false;
	this->_received_bytes = 0;
	_respond_path.clear();
	_response_body.clear();
	_response.clear();
	setChunked();
	_request.setUserIP(_addr);
	std::map<std::string, Location>::iterator loc_it;
	std::string uri = request.getUri();
	size_t pos = uri.find_last_of("/");
	if (pos == std::string::npos)
		pos = 0;
	if (uri.find_first_of(".", pos) == std::string::npos)
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
	while (!uri.empty())
	{
		if (uri.length() > 1 && uri[uri.length() - 1] == '/')
			uri.erase(uri.length() - 1);
		std::map<std::string, std::string>::iterator red_it = this->getConfig().getRedirection().find(uri);
		if (red_it != this->getConfig().getRedirection().end())
		{
			size += red_it->second.size() - uri.size();
			uri = red_it->second;
			loc_it = this->getConfig().get_location().find(red_it->second);
			if (loc_it == this->getConfig().get_location().end())
			{
				_is_redirect = true;
				this->_request.setURI(red_it->second);
				return true; 
			}

		}
		else
			loc_it = this->getConfig().get_location().find(uri);
		if (loc_it != this->getConfig().get_location().end())
		{
			this->_location = loc_it->second;
			if (this->_is_dir)
			{
				if (!this->_list_dir && uri.size() == size)
				{
					if (!this->_location.get_index().empty())
					{
						this->_request.setURI(this->_location.get_root() + this->_location.get_index());
						this->_is_dir = false;
					}
				}
				else
					this->_request.setURI(this->_location.get_root() + &request.getUri()[pos + 1]);
			}
			else
				this->_request.setURI(this->_location.get_root() + &request.getUri()[pos + 1]);
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
	std::ifstream	pathTest(_respond_path.c_str());
	if (pathTest.fail() == true)
		response_stream << createError(404);
	if  (remove(_respond_path.c_str()) == -1)
		response_stream << createError(403);
	else
	{
		response_stream << HTTP_204 << _types.get_content_type(".txt");
		this->_status_code = 204;
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

/**
 * @brief 
 * 
 * @param errorNumber Standard error code
 * @param config Needs a config to access paths, should be NULL if error is pre-config creation
 * @return std::string 
 */
std::string	Response::createError(int errorNumber)
{
	std::string			response_body;
	std::string			errorName;
	std::ostringstream	response_stream;
	std::string			error_path = getErrorPath(errorNumber, errorName);
	std::ifstream error(error_path.c_str());

	this->_status_code = errorNumber;
	if(!error.is_open())
		std::cerr << RED << "error opening " << errorNumber << " file at " << error_path << RESET << std::endl;
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

std::string Response::getErrorPath(int &errorNumber, std::string& errorName)
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
	error_path = this->_config.get_error_path(errorNumber);
	return (error_path);
}

bool Response::checkCGI()
{
	size_t pos = this->_request.getUri().find_last_of(".");
	size_t pos2 = this->_request.getUri().find_last_of("/");
	if (this->getConfig().getIntrPath().empty())
		return false;
	if (pos != std::string::npos)
	{
		if (pos2 != std::string::npos && pos < pos2)
		{
			this->_is_dir = true;
			return false;
		}
		this->_ext = this->_request.getUri().substr(pos);
		std::map<std::string, std::string>::iterator it = this->getConfig().getIntrPath().find(this->_ext);
		if (it != this->getConfig().getIntrPath().end())
			return true;
	}
	this->_ext.clear();
	return false;
}

bool	Response::checkPermissions()
{
	std::string path = this->getRequest().getUri();
	if (path.size() == 1)
		path = this->_location.get_root() + this->_location.get_index();
	else
		path = this->_location.get_root() + &path[1];
	return dir_exists(path);
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

/**
 * @brief Checks if a directory exists at path
        using the stat() system call.
 * 
 * @param path path to check
 * @return true or false
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

/**
 * @brief Returns a string containing an HTML directory listing
 * 
 * @param uri path to directory
 * @return std::string containing HTML
*/
std::string Response::directoryListing(std::string uri)
{
    DIR *dir;
    struct dirent *ent;

	if (!directoryExists(uri.c_str()))
		return (Response::createError(404));
	
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

/** 
 * @brief Checks if a directory exists at a given path.
 * 
 * @param dirName_in path to directory
 * @return true or false
*/
bool Response::dir_exists(const std::string& dirName_in)
{
	struct stat info;

	int ret = stat(dirName_in.c_str(), &info);
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
	return (this->getRequest().getContentLength() - this->_received_bytes);
}

std::string &Response::getExt()
{
	return this->_ext;
}

void	Response::setChunked()
{
	if (this->_request.get_single_header("transfer-encoding") == "chunked")
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

std::string Response::redirect(std::string uri)
{
	std::ostringstream message;

	message << "HTTP/1.1 307 Temporary Redirect\r\n";
	message << "Location: " << uri << "\r\n\r\n";
	return (message.str());
}

std::string	Response::getResponseBuff()
{
	return (_response);
}

void Response::setResponseBuff(std::string response)
{
	_response = response;
}

void Response::setToClose()
{
	this->_to_close = true;
}

void Response::revertCGI()
{
	this->_is_cgi = false;
	this->_is_complete = false;
}
