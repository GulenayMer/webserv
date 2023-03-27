#include "../include/Config.hpp"


Config::Config(): _error_code(0)
{
	this->set_port(80);
	this->set_host(inet_addr("127.0.0.1"));
	this->set_server_name("default");
	this->set_client_max_body_size(1048576);
	this->set_autoindex(false);
	this->set_root("");
	this->set_index("");
}

Config::Config(const Config& obj)
{
	*this = obj;
}

Config &Config::operator=(const Config& obj)
{
	if (this != &obj) {
		this->_port = obj._port;
		this->_host = obj._host;
		this->_server_name = obj._server_name;
		this->_default_error = obj._default_error;
		this->_client_max_body_size = obj._client_max_body_size;
		this->_autoindex = obj._autoindex;
		this->_root = obj._root;
		this->_index = obj._index;
		this->_error_code = obj._error_code;
		this->_cgi = obj._cgi;
		this->_location = obj._location;
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

in_addr					&Config::get_host()
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

configCGI									&Config::get_cgi()
{
	return this->_cgi;
}

int 								Config::get_error_code()
{
	return this->_error_code;
}


// Setters

void 					Config::set_error_code(int error_code)
{
	this->_error_code = error_code;
}

void					Config::set_port(u_int16_t port)
{
	this->_port = port;
}

void					Config::set_host(in_addr_t host)
{
	this->_host.s_addr = host;
}

void					Config::set_server_name(std::string server_name)
{
	this->_server_name = server_name;
}

void					Config::set_default_error(int i, std::string default_error)
{
	std::pair<int, std::string> p = std::make_pair(i, default_error);
	this->_default_error.insert(p);
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
	Location location = Location(config_file, line);
	this->set_error_code(location.get_error_code());
	this->_location.insert(std::make_pair(key, location));
}

void					Config::set_cgi(std::ifstream& config_file, std::string line)
{
	this->_cgi = configCGI(config_file, line);
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
	if (this->get_index().size() == 0 || file_exists(this->get_root() + this->get_index()) == false) {
		this->set_error_code(21);
		throw std::logic_error(INVALID_INDEX);
	}
	
	// default error check - files provided by paths exist 
	if (this->get_default_error().size() == 0)
		this->get_default_error().insert(std::make_pair(404, "/error/404_NotFound.html"));
	std::map<int, std::string> error = this->get_default_error();
	for (std::map<int, std::string>::iterator it = error.begin(); it != error.end(); it++) {
		if (file_exists(this->get_root() + it->second) == false) {
			this->set_error_code(22);
			throw std::logic_error(INVALID_ERROR_CHECK);
		}
	}

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
	this->set_error_code(this->get_cgi().cgi_check());
	int cgi_error = this->get_error_code();
	switch (cgi_error) {
		case 27:
			throw std::logic_error(INVALID_CGI_ROOT);
		case 28:
			throw std::logic_error(INVALID_PROGRAM_PATH);
	}
}

std::ostream& operator<<(std::ostream& os, Config& config)
{
	os << "port: " << config.get_port() << std::endl;
	os << "host: " << inet_ntoa(config.get_host()) << std::endl;
	
	std::map<int, std::string> errors = config.get_default_error();
	std::map<int, std::string>::const_iterator e_it = errors.begin();
	std::cout << "error paths: " << std::endl;
	while (e_it != errors.end()) {
		os << e_it->first << " : " << e_it->second << std::endl;
		e_it++;
	}
	
	os << "server name: "<< config.get_server_name() << std::endl;
	os << "client max body size: " << config.get_client_max_body_size() << std::endl;
	os << "root: " << config.get_root() << std::endl;
	os << "index: " << config.get_index() << std::endl;
	
	std::map<std::string, Location> locations = config.get_location();
	std::map<std::string, Location>::iterator l_it = locations.begin();
	while (l_it != locations.end()) {
		os << std::endl;
		os << "location: " <<l_it->first << std::endl;
		os << l_it->second << std::endl;
		l_it++;
	}
	
	os << "cgi config: " << std::endl;
	os << config.get_cgi() << std::endl;
	return (os);
}
