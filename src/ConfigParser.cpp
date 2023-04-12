#include "../include/ConfigParser.hpp"

ConfigParser::ConfigParser()
{
	std::ifstream in_file;
	
	this->_error_code = 0;
	this->_n_servers = 0;
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
	
	this->_error_code = 0;
	this->_n_servers = 0;
	
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

std::vector<Config>	&ConfigParser::get_configs()
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

	while (getline(config_file, line) && !this->_error_code) {
		line = remove_comments(line);
		if (line.find_first_not_of(" \r\t\b\f") == std::string::npos)
			continue;
		if ((line.find("server") != std::string::npos && check_def_format("server", line) && line.find("{") != std::string::npos) && line.find("}") != std::string::npos)
			return false;
		else if ((line.find("server") != std::string::npos && check_def_format("server", line) && line.find("{") != std::string::npos) && context == 0) {
			this->_n_servers++;
			context += 1;
			this->_configs.push_back(Config());
		}
		else if (context == 0) {
			this->set_error_code(2);
			return false;
		}
		if ((context && line.find(LISTEN) != std::string::npos) && check_def_format(LISTEN, line))
			this->clean_listen(line);
		// else if ((context && line.find(HOST) != std::string::npos) && check_def_format(HOST, line))
		// 	this->clean_host(line);
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
		else if ((line.find(CGI_PATH) != std::string::npos) && check_def_format(CGI_PATH, line))
			this->clean_path(line);
		else if ((line.find(CGI_EXT) != std::string::npos) && check_def_format(CGI_EXT, line))
			this->clean_ext(line);
		/*
		
			go into function and separate location config
			keep track of context
		
		*/
		if (line.find("location") != std::string::npos && line.find("{") != std::string::npos) {
			context += 1;
			this->get_config(this->get_n_servers() - 1).set_location(config_file, line);
			this->set_error_code(this->get_config(this->get_n_servers() - 1).get_error_code());
			if (this->get_error_code() != 0)
				return false;
			context -= 1;
		}
        if (line.find("}") != std::string::npos)
        {
            context -= 1;
            if (context == 0)
            {
                addToExtMap();
                this->get_config(this->_n_servers - 1).combineHost();
            }
            else if (context < 0)
            {
                // TODO throw or break
				// throw std::runtime_error("Invalid configuration file");
            }
        }
	}
	// std::map<int, std::string>::iterator it = this->get_config(this->get_n_servers() - 1).get_default_error().begin();
	// while (it != this->get_config(this->get_n_servers() - 1).get_default_error().end())
	// {
	// 	std::cout << it->first << std::endl;
	// 	std::cout << it->second << std::endl;
	// 	++it;
	// }
	if (context != 0 && this->get_error_code() != 0)
		return false;
	return true;
}

void ConfigParser::clean_listen(std::string line)
{
	line = find_int(line, 1);
	if (line.empty())
	{
		this->get_config(this->_n_servers - 1).set_port(0);
		return (this->set_error_code(3));
	}
	this->get_config(this->_n_servers - 1).setHost(line);
	this->get_config(this->_n_servers - 1).set_port(to_int(line.c_str()));
}

void ConfigParser::clean_server_name(std::string line)
{
    std::string word = get_word(line, 1);
    std::string word2 = get_word(line, 2);
    bool	w_ip = true;
    if (word.empty())
        return (this->set_error_code(4));
    for (size_t i = 0; i < word.length(); i++)
    {
        if (!isdigit(word[i]) && word[i] != '.')
        {
            w_ip = false;
            if (word2.empty())
                word2 = findIP(word);
            break;
        }
    }
    if (!w_ip && !word2.empty())
    {
        for (size_t i = 0; i < word2.length(); i++)
        {
            if (!isdigit(word2[i]) && word2[i] != '.')
                return (this->set_error_code(4));
        }
    }
    else if (!w_ip)
        return (this->set_error_code(4));
    if (w_ip)
    {
        if (word2.empty())
            this->get_config(this->get_n_servers() - 1).set_server_name(word);
        else
            this->get_config(this->get_n_servers() - 1).set_server_name(word2);
        this->get_config(this->get_n_servers() - 1).set_addr(inet_addr(word.c_str()));
    }
    else if (!w_ip)
    {
        this->get_config(this->get_n_servers() - 1).set_server_name(word);
        this->get_config(this->get_n_servers() - 1).set_addr(inet_addr(word2.c_str()));
    }
}

void ConfigParser::clean_error_page(std::string line)
{
	if (this->get_config(this->get_n_servers() - 1).get_root().empty())
		return this->set_error_code(5);
	std::string error;
	error = find_int(line, 1);
	if (error.empty())
		return this->set_error_code(5);
	std::size_t pos;
	std::size_t pos2;
	pos = line.find(error);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos2 = line.find_first_of(" \r\t\b\f;", pos);
	if (pos == std::string::npos)
		return this->set_error_code(5);
	if (line[pos] == '/')
		pos += 1;
	if (pos2 != std::string::npos)
		line.erase(pos2);
	this->get_config(this->get_n_servers() - 1).set_default_error(to_int(error), this->get_config(this->get_n_servers() - 1).get_root() + &line[pos]);
}

// void ConfigParser::clean_server_name(std::string line)
// {
// 	line = get_value(line);
// 	if (line.size() == 0)
// 		this->set_error_code(6);
// 	this->get_config(this->get_n_servers() - 1).set_server_name(line);
// }

void ConfigParser::clean_client_max_body_size(std::string line)
{
	line = find_int(line, 1);
	if (line.size() == 0)
		this->set_error_code(7);
	this->get_config(this->_n_servers - 1).set_client_max_body_size(to_int(line.c_str()));
}

void ConfigParser::clean_autoindex(std::string line)
{
	if (line.find("on") != std::string::npos)
		this->get_config(this->_n_servers - 1).set_autoindex(true);
	else if (line.find("off") != std::string::npos)
		this->get_config(this->_n_servers - 1).set_autoindex(false);
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
	this->get_config(this->_n_servers - 1).set_root(line);
}

void ConfigParser::clean_index(std::string line)
{
	line = get_value(line);
	if (line.size() == 0)
		this->set_error_code(10);
	this->get_config(this->get_n_servers() - 1).set_index(line);
}

void ConfigParser::clean_path(std::string line)
{
	size_t pos = 0;
	size_t pos2 = 0;
	line = remove_end(line, ';');
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	while (pos != std::string::npos) {
		pos2 = line.find_first_of(" \r\t\b\f", pos);
		this->_path.push_back(line.substr(pos, pos2 - pos));
		pos = line.find_first_not_of(" \r\t\b\f", pos2);
	}
}

void			ConfigParser::clean_ext(std::string line)
{
	size_t pos = 0;
	size_t pos2 = 0;
	line = remove_end(line, ';');
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	while (pos != std::string::npos) {
		pos = line.find_first_not_of(" \r\t\b\f", pos);
		pos2 = line.find_first_of(" \r\t\b\f", pos);
		this->_ext.push_back(line.substr(pos, pos2 - pos));
		pos = pos2;
	}
}

void	ConfigParser::addToExtMap()
{
	std::vector<std::string>::iterator it1 = this->_ext.begin();
	std::vector<std::string>::iterator it2 = this->_path.begin();
	while (it1 != this->_ext.end() && it2 != this->_path.end())
	{
		this->get_config(this->_n_servers - 1).setIntrPath(*it1, *it2);
		++it1;
		++it2;
	}
	if (it1 != this->_ext.end() || it2 != this->_path.end())
		this->set_error_code(12);
	this->_ext.clear();
	this->_path.clear();
}

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

std::string ConfigParser::findIP(std::string &word)
{
	std::ifstream ifile;
	char	buff[256];

	ifile.open("/etc/hosts");
	if (!ifile.is_open())
		return "";
	while (ifile.getline(buff, 256))
	{
		std::string str(buff);
		if (str.find(word) != std::string::npos)
		{
			size_t pos = str.find_first_of(" \f\n\r\t\v");
			if (pos != std::string::npos)
				return str.substr(0, pos);
		}
	}
	return "";
}
