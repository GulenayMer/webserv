#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser()
{
	std::ifstream in_file;
	
	this->set_error_code(0);
	this->set_n_servers(0);
	//this->_configs = new std::vector<Config>;
	
	// Check if config file exists
	in_file.open("./webserv.config", std::ios::in);
	if (in_file.fail() == true) {
		this->exit_with_error(1, in_file);
		return;
	}
	
	// Check if server context exists
	if (this->check_server_context(in_file) == false)
		this->exit_with_error(this->get_error_code(), in_file);
	in_file.close();
}

ConfigParser::ConfigParser(std::string config_file)
{
	std::ifstream in_file;
	
	this->set_error_code(0);
	this->set_n_servers(0);
	//this->_configs = new std::vector<Config>;
	
	// Check if config file exists
	in_file.open(config_file.c_str(), std::ios::in);
	if (in_file.is_open() == false){
		this->exit_with_error(28, in_file);
		return;
	};
	
	// Check if server context exists
	if (this->check_server_context(in_file) == false)
		this->exit_with_error(this->get_error_code(), in_file);
	in_file.close();
}

ConfigParser::~ConfigParser()
{
	//  for (size_t i = 0; i < this->_configs.size(); i++)
	//  	delete this->_configs[i];
}

std::vector<Config>	ConfigParser::get_configs()
{
	return this->_configs;
}

Config& ConfigParser::get_config(int i)
{
	return this->_configs.at(i);
}

int ConfigParser::get_error_code()
{
	return this->_error_code;
}

int ConfigParser::get_n_servers()
{
	return this->_n_servers;
}

void ConfigParser::set_error_code(int error_code)
{
	this->_error_code = error_code;
}

void ConfigParser::set_n_servers(int i)
{
	this->_n_servers = i;
}

bool ConfigParser::check_server_context(std::ifstream& config_file)
{
	std::string line;
	int context = 0;

	while (getline(config_file, line) && this->get_error_code() == 0) {
		line = remove_comments(line);
		if (line.find_first_not_of(" \r\t\b\f") == std::string::npos)
			continue;
		
		if ((line.find("server") != std::string::npos && check_def_format("server", line) && line.find("{") != std::string::npos) && line.find("}") != std::string::npos)
			return false;
		else if ((line.find("server") != std::string::npos && check_def_format("server", line) && line.find("{") != std::string::npos) && context == 0) {
			this->set_n_servers(this->get_n_servers() + 1);
			context += 1;
			this->_configs.push_back(Config());
		}
		else if (context == 0) {
			this->set_error_code(2);
			return false;
		}

		if ((context && line.find(LISTEN) != std::string::npos) && check_def_format(LISTEN, line))
			this->clean_listen(line);
		else if ((context && line.find(HOST) != std::string::npos) && check_def_format(HOST, line))
			this->clean_host(line);
		else if ((context && line.find(ROOT) != std::string::npos) && check_def_format(ROOT, line))
			this->clean_root(line);
		else if ((context && line.find(INDEX) != std::string::npos) && check_def_format(INDEX, line))
			this->clean_index(line);
		else if ((context && line.find(ERROR_PAGE) != std::string::npos) && check_def_format(ERROR_PAGE, line))
			this->clean_error_page(line);
		else if ((context && line.find(SERVER_NAME) != std::string::npos) && check_def_format(SERVER_NAME, line))
			this->clean_server_name(line);
		else if ((context && line.find(CLIENT_MAX_BODY) != std::string::npos) && check_def_format(CLIENT_MAX_BODY, line))
			this->clean_client_max_body_size(line);
		else if ((context && line.find(AUTOINDEX) != std::string::npos) && check_def_format(AUTOINDEX, line))
			this->clean_autoindex(line);
		/*
		
			go into function and separate location config
			keep track of context
		
		*/
		if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
			context += 1;
			if (line.find("cgi-bin") != std::string::npos) {
				this->get_config(this->get_n_servers() - 1).set_cgi(config_file, line);
				int err = this->get_config(this->get_n_servers() - 1).get_cgi().get_error_code();
				if (err != 0)
					this->set_error_code(err);
			}
			else {
				this->get_config(this->get_n_servers() - 1).set_location(config_file, line);
				this->set_error_code(this->get_config(this->get_n_servers() - 1).get_error_code());
			}
			if (this->get_error_code() != 0)
				return false;
			context -= 1;
		}
		if (line.find("}") != std::string::npos && context > 0)
			context -= 1;
	}

	if (context != 0 && this->get_error_code() != 0)
		return false;
	return true;
}

void ConfigParser::clean_listen(std::string line)
{
	line = find_int(line, 1);
	if (line.empty())
	{
		this->get_config(this->get_n_servers() - 1).set_port(0);
		return (this->set_error_code(3));
	}
	this->get_config(this->get_n_servers() - 1).set_port(to_int(line.c_str()));
}

void ConfigParser::clean_host(std::string line)
{
	line = get_word(line, 1);
	if (line.empty())
		return (this->set_error_code(4));
	for (size_t i = 0; i < line.length(); i++)
	{
		if (!isdigit(line[i]) && line[i] != '.')
			return (this->set_error_code(4));
	}
	this->get_config(this->get_n_servers() - 1).set_host(inet_addr(line.c_str()));
}

void ConfigParser::clean_error_page(std::string line)
{
	std::string error;
	error = find_int(line, 1);
	if (error.empty())
		this->set_error_code(5);
	std::size_t pos;
	std::size_t pos2;
	pos = line.find(error);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos2 = line.find_first_of(" \r\t\b\f", pos);
	pos2 = line.find_first_not_of(" \r\t\b\f", pos2);
	if (pos == std::string::npos)
		return this->set_error_code(5);
	if (line[pos] == '/')
		pos += 1;
	line.erase(0, pos);
	if (pos2 != std::string::npos)
		line.erase(pos2);
	line = remove_end(line, ';');
	this->get_config(this->get_n_servers() - 1).set_default_error(to_int(error), line);
}

void ConfigParser::clean_server_name(std::string line)
{
	line = get_value(line);
	if (line.size() == 0)
		this->set_error_code(6);
	this->get_config(this->get_n_servers() - 1).set_server_name(line);
}

void ConfigParser::clean_client_max_body_size(std::string line)
{
	line = find_int(line, 1);
	if (line.size() == 0)
		this->set_error_code(7);
	this->get_config(this->get_n_servers() - 1).set_client_max_body_size(to_int(line.c_str()));
}

void ConfigParser::clean_autoindex(std::string line)
{
	if (line.find("on") != std::string::npos)
		this->get_config(this->get_n_servers() - 1).set_autoindex(true);
	else if (line.find("off") != std::string::npos)
		this->get_config(this->get_n_servers() - 1).set_autoindex(false);
	else	
		this->set_error_code(8);
}

void ConfigParser::clean_root(std::string line)
{
	line = get_value(line);
	if (line.size() == 0)
		this->set_error_code(9);
	if (line[line.size() - 1] != '/')
		line = line + "/";
	this->get_config(this->get_n_servers() - 1).set_root(line);
}

void ConfigParser::clean_index(std::string line)
{
	line = get_value(line);
	if (line.size() == 0)
		this->set_error_code(10);
	this->get_config(this->get_n_servers() - 1).set_index(line);
}

// void			ConfigParser::clean_cgi_ext(std::string line, configCGI &cgi)
// {
// 	size_t pos = 0;
// 	size_t pos2 = 0;
// 	line = remove_end(line, ';');
// 	pos = line.find_first_not_of(" \r\t\b\f", pos);
// 	pos = line.find_first_of(" \r\t\b\f", pos);
// 	pos = line.find_first_not_of(" \r\t\b\f", pos);
// 	while (pos != std::string::npos) {
// 		pos = line.find_first_not_of(" \r\t\b\f", pos);
// 		pos2 = line.find_first_of(" \r\t\b\f", pos);
// 		cgi.set_ext(line.substr(pos, pos2 - pos));
// 		pos = pos2;
// 	}
// 	if (cgi.get_path().empty())
// 		this->set_error_code(15);
// }

int ConfigParser::exit_with_error(int err_code, std::ifstream& in_file)
{
	if (err_code == 1)
		std::cerr << RED << NO_DEFAULT_CONFIG << RESET << std::endl;
	else if (err_code == 2)
		std::cerr << RED << NO_VALID_SERVER << RESET << std::endl;
	else if (err_code == 3)
		std::cerr << RED << NO_VALID_PORT << RESET << std::endl;
	else if (err_code == 4)
		std::cerr << RED << NO_VALID_HOST << RESET << std::endl;
	else if (err_code == 5)
		std::cerr << RED << NO_VALID_ERROR_PAGE << RESET << std::endl;
	else if (err_code == 6)
		std::cerr << RED << NO_VALID_SERVER_NAME << RESET << std::endl;
	else if (err_code == 7)
		std::cerr << RED << NO_VALID_CLIENT_MAX_BODY_SIZE << RESET << std::endl;
	else if (err_code == 8)
		std::cerr << RED << NO_VALID_AUTOINDEX << RESET << std::endl;
	else if (err_code == 9)
		std::cerr << RED << NO_VALID_ROOT << RESET << std::endl;
	else if (err_code == 10)
		std::cerr << RED << NO_VALID_INDEX << RESET << std::endl;
	else if (err_code == 11)
		std::cerr << RED << NO_VALID_LOCATION << RESET << std::endl;
	else if (err_code == 12)
		std::cerr << RED << NO_VALID_CGI << RESET << std::endl;
	else if (err_code == 13)
		std::cerr << RED << NO_VALID_METHODS<< RESET << std::endl;
	else if (err_code == 14)
		std::cerr << RED << NO_VALID_CGI_PATH << RESET << std::endl;
	else if (err_code == 15)
		std::cerr << RED << NO_VALID_CGI_EXT << RESET << std::endl;
	else if (err_code == 28)
		std::cerr << RED << NO_VALID_CONFIG_FILE << RESET << std::endl;
	in_file.close();
	this->set_error_code(err_code);
	return(err_code);
}
