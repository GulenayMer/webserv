#include "../include/configCGI.hpp"


configCGI::configCGI()
{
	this->_error_code = 0;
}

configCGI::configCGI(std::ifstream& config_file, std::string line)
{
	int	exit_context = 0;

	_error_code = 0;
	while (!exit_context && getline(config_file, line)) {
		if (line.find("}") != std::string::npos)
			exit_context = 1;
		if ((line.find(ROOT) != std::string::npos) && check_def_format(ROOT, line))
			this->set_root(get_value(line));
		else if ((line.find(CGI_PATH) != std::string::npos) && check_def_format(CGI_PATH, line))
			clean_path(line);
		else if ((line.find(CGI_EXT) != std::string::npos) && check_def_format(CGI_EXT, line))
			clean_ext(line);
	}
	if (!exit_context || (this->get_path().empty() || this->get_ext().empty() || this->get_root().empty()))
		set_error_code(12);	
}

void			configCGI::clean_ext(std::string line)
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
		this->set_ext(line.substr(pos, pos2 - pos));
		pos = pos2;
	}
	if (this->get_path().empty())
		this->set_error_code(15);
}

configCGI::configCGI(const configCGI &src)
{
	*this = src;
}

configCGI &configCGI::operator=(const configCGI &src)
{
	if (this != &src) {
		this->_root = src._root;
		this->_path = src._path;
		this->_ext = src._ext;
		this->_error_code = src._error_code;
	}
	return *this;
}


const std::string									&configCGI::get_root() const
{
	return this->_root;
}

const std::map<std::string, std::string>			&configCGI::get_path() const
{
	return this->_path;
}

const std::vector<std::string>					&configCGI::get_ext() const
{
	return this->_ext;
}

const int &configCGI::get_error_code() const
{
	return this->_error_code;
}

void configCGI::set_error_code(int error_code)
{
	this->_error_code = error_code;
}

void										configCGI::set_root(std::string root)
{
	this->_root = root;
}

void										configCGI::set_path(std::string program, std::string path)
{
	this->_path.insert(std::make_pair(program, path));
}

void										configCGI::set_ext(std::string ext)
{
	this->_ext.push_back(ext);
}

int											configCGI::cgi_check()
{
	// root value exists and given directory exists
	if (this->get_root().size() == 0 || dir_exists(this->get_root()) == false) {
		return 27;
	}
	
	// provided programm paths are vealid
	std::map<std::string, std::string> paths = this->get_path();
	std::map<std::string, std::string>::const_iterator p_it = paths.begin();
	while (p_it != paths.end()) {
		if (file_exists(p_it->second) == false) {
			return 28;
		}
		p_it++;
	}
	return EXIT_SUCCESS;
}

void									configCGI::clean_path(std::string line)
{
	std::string temp;
	std::string temp2;
	size_t pos = 0;
	size_t pos2 = 0;
	size_t pos3 = 0;
	line = remove_end(line, ';');
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	while (pos != std::string::npos) {
		pos3 = line.find_first_of(" \r\t\b\f", pos);
		pos2 = line.find_last_of("/", pos3);
		temp =  line.substr(pos2 + 1, pos3 - pos2 - 1);
		temp2 = line.substr(pos, pos2 + 1 - pos + (temp).size());
		this->set_path(temp, temp2);
		pos = line.find_first_not_of(" \r\t\b\f", pos3);
	}
	if (this->get_path().empty())
		this->set_error_code(14);
}

std::ostream &operator<<(std::ostream &os, const configCGI &cgi)
{
	os << "root: " << cgi.get_root() << std::endl;
	
	std::map<std::string, std::string> paths = cgi.get_path();
	std::map<std::string, std::string>::const_iterator p_it = paths.begin();
	std::cout << "program paths: " << std::endl;
	while (p_it != paths.end()) {
		os << p_it->first << " : " << p_it->second << std::endl;
		p_it++;
	}
	std::cout << "allowed extensions: " << std::endl;
	for (size_t i = 0; i < cgi.get_ext().size(); i++) {
		std::cout << cgi.get_ext()[i] << std::endl;
	}
	
	return os;
}
