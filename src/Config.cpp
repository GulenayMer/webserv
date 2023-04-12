#include "../include/Config.hpp"


Config::Config(): _error_code(0)
{
	this->set_port(80);
	this->set_addr(inet_addr("127.0.0.1"));
	this->set_server_name("default");
	this->set_client_max_body_size(1048576);
	this->set_autoindex(false);
	this->create_default_errors();
}

Config::Config(const Config &obj)
{
	*this = obj;
}

Config &Config::operator=(const Config &obj)
{
	if (this != &obj) {
		this->_port = obj._port;
		this->_addr = obj._addr;
		this->_server_name = obj._server_name;
		this->_host = obj._host;
		this->_default_error = obj._default_error;
		this->_client_max_body_size = obj._client_max_body_size;
		this->_autoindex = obj._autoindex;
		this->_root = obj._root;
		this->_index = obj._index;
		this->_error_code = obj._error_code;
		this->_location = obj._location;
		this->_redirection = obj._redirection;
		this->_intr_paths = obj._intr_paths;
	}
	return *this;
}

Config::~Config()
{

}

// Getters

u_int16_t					&Config::get_port()
{
	return this->_port;
}

in_addr					&Config::get_addr()
{
	return this->_addr;
}

std::string		&Config::getHost()
{
	return this->_host;
}

std::string					&Config::get_server_name()
{
	return this->_server_name;
}

std::map<int, std::string>	&Config::get_default_error()
{
	return this->_default_error;
}

std::string							&Config::get_error_path(int error)
{
	
	return this->get_default_error().at(error);
}

size_t 						&Config::get_client_max_body_size()
{
	return this->_client_max_body_size;
}

bool						&Config::get_autoindex()
{
	return this->_autoindex;
}

std::string					&Config::get_root()
{
	return this->_root;
}

std::string					Config::get_index()
{
	return this->_index;
}

std::map<std::string, Location>		&Config::get_location()
{
	return this->_location;
}

// Location							&Config::find_location(std::string location)
// {
// 	std::map<std::string, Location>::iterator it = _location.find(location);
// 	if (it != _location.end())
// 		return (&it->second);
// 	return (NULL);
// }

int 								Config::get_error_code()
{
	return this->_error_code;
}

std::map<std::string, std::string>	&Config::getIntrPath()
{
	return this->_intr_paths;
}

void								Config::setIntrPath(std::string &ext, std::string &path)
{
	this->_intr_paths.insert(std::map<std::string, std::string>::value_type(ext, path));
}

// Setters

void 					Config::set_error_code(int error_code)
{
	this->_error_code = error_code;
}

void					Config::set_port(uint16_t port)
{
	this->_port = port;
}

void					Config::set_addr(in_addr_t addr)
{
	this->_addr.s_addr = addr;
}

void	Config::setHost(std::string &host)
{
	this->_host = host;
}

void					Config::set_server_name(std::string server_name)
{
	this->_server_name = server_name;
}

void					Config::set_default_error(int i, std::string default_error)
{
	this->_default_error[i] = default_error;
	// if (this->_default_error.find(i) != this->_default_error.end()) {
	// 	std::cout << i << std::endl;
	// 	std::cout << default_error << std::endl;
	// 	std::cout << "found" << std::endl;
	// 	this->_default_error.find(i)->second = this->get_root() + default_error;
	// }
	// else {
	// 	std::cout << i << std::endl;
	// 	std::cout << default_error << std::endl;
	// 	std::cout << "not found" << std::endl;
	// 	std::pair<int, std::string> p = std::make_pair(i, default_error);
	// 	this->_default_error.insert(p);
	// }
}

void 					Config::set_client_max_body_size(int clien_max_body_size)
{
	this->_client_max_body_size = clien_max_body_size;
}

void					Config::set_autoindex(bool autoindex)
{
	this->_autoindex = autoindex;
}

void					Config::set_root(std::string root)
{
	this->_root = root;
}

void					Config::set_index(std::string index)
{
	this->_index = index;
}

void					Config::set_location(std::ifstream& config_file, std::string line)
{
	std::string key = get_value(line);
	Location location(config_file, line);
	this->set_error_code(location.get_error_code());
	// std::map<std::string, Location>::iterator it = this->_location.find(location.get_redirection());
	if (!location.get_redirection().empty())
	{
		this->_redirection.insert(std::make_pair(key, location.get_redirection()));
	}
	else
		this->_location.insert(std::make_pair(key, location));
}

//TODO create test cases
void						Config::check_config()
{
	// root value exists and given directory exists
	if (this->get_root().size() == 0 || dir_exists(this->get_root()) == false) {
		this->set_error_code(20);
		throw std::logic_error(INVALID_ROOT);
	}
	
	// index value exists and given file exists
	if (this->get_index().size() == 0 || !file_exists(this->get_root() + this->get_index())) {
		this->set_error_code(21);
		throw std::logic_error(INVALID_INDEX);
	}
	
	// default error check - files provided by paths exist 
	// if (this->get_default_error().size() == 0)
	// 	this->get_default_error().insert(std::make_pair(404, "/error/404_NotFound.html"));
	// std::map<int, std::string> error = this->get_default_error();
	// for (std::map<int, std::string>::iterator it = error.begin(); it != error.end(); it++) {
	// 	if (file_exists(it->second) == false) {
	// 		std::cout << "NOT FOUND: " << it->second << std::endl;
	// 		this->set_error_code(22);
	// 		throw std::logic_error(INVALID_ERROR_CHECK);
	// 	}
	// }

	// Location check
	std::map<std::string, Location> location = this->get_location();
	for (std::map<std::string, Location>::iterator it = location.begin(); it != location.end(); it++) {
		int error_code = it->second.check_location();
		if (error_code != EXIT_SUCCESS) {
			this->set_error_code(error_code);
			switch (this->get_error_code()) {
				case 23:
					throw std::logic_error(INVALID_LOCATION_ROOT);
				case 24:
					throw std::logic_error(INVALID_LOCATION_INDEX);
				case 25:
					throw std::logic_error(INVALID_LOCATION_REDIRECTION);
				case 26:
					throw std::logic_error(INVALID_METHODS);
			}
		}
	}
	// CGI check
	std::map<std::string, std::string>::const_iterator p_it = _intr_paths.begin();
	while (p_it != _intr_paths.end()) {
		if (!file_exists(p_it->second)) {
			throw std::logic_error(INVALID_PROGRAM_PATH);
		}
		p_it++;
	}
}

void	Config::create_default_errors()
{
	this->set_default_error(400, "docs/www/default_error/400_BadRequest.html");
	this->set_default_error(403, "docs/www/default_error/403_Forbidden.html");
	this->set_default_error(404, "docs/www/default_error/404_NotFound.html");
	this->set_default_error(405, "docs/www/default_error/405_MethodNotAllowed.html");
	this->set_default_error(406, "docs/www/default_error/406_NotAcceptable.html");
	this->set_default_error(408, "docs/www/default_error/408_RequestTimeout.html");
	this->set_default_error(411, "docs/www/default_error/411_LengthRequired.html");
	this->set_default_error(413, "docs/www/default_error/413_PayloadTooLarge.html");
	this->set_default_error(414, "docs/www/default_error/414_URITooLarge.html");
	this->set_default_error(415, "docs/www/default_error/415_UnsupportedMediaType.html");
	this->set_default_error(418, "docs/www/default_error/418_Imateapot.html");
	this->set_default_error(429, "docs/www/default_error/429_TooManyRequests.html");
	this->set_default_error(500, "docs/www/default_error/500_InternalServer.html");
	this->set_default_error(501, "docs/www/default_error/501_NotImplemented.html");
	this->set_default_error(502, "docs/www/default_error/502_BadGateway.html");
	this->set_default_error(503, "docs/www/default_error/503_ServiceUnavailable.html");
	this->set_default_error(504, "docs/www/default_error/504_GatewayTimeout.html");
	this->set_default_error(505, "docs/www/default_error/505_HTTPVersionNotSupported.html");
}

std::map<std::string, std::string> &Config::getRedirection()
{
	return this->_redirection;
}

void	Config::combineHost()
{
	this->_host.insert(0, ":");
	this->_host.insert(0, this->_server_name);
}
