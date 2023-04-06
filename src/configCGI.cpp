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
	addToMap();
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
		this->_ext.push_back(line.substr(pos, pos2 - pos));
		pos = pos2;
	}
	if (this->_ext.empty())
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
		this->_intr_paths = src._intr_paths;
	}
	return *this;
}


const std::string									&configCGI::get_root() const
{
	return this->_root;
}

const std::map<std::string, std::string>			&configCGI::get_path() const
{
	return this->_intr_paths;
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

void										configCGI::set_path(std::string ext, std::string path)
{
	this->_intr_paths.insert(std::map<std::string, std::string>::value_type(ext, path));
}

int											configCGI::cgi_check()
{
	// root value exists and given directory exists
	if (this->get_root().size() == 0 || dir_exists(this->get_root()) == false) {
		return 27;
	}
	
	// provided program paths are valid
	std::map<std::string, std::string>::const_iterator p_it = _intr_paths.begin();
	while (p_it != _intr_paths.end()) {
		if (!file_exists(p_it->second)) {
			return 28;
		}
		p_it++;
	}
	return EXIT_SUCCESS;
}

void									configCGI::clean_path(std::string line)
{
	std::string temp;
	size_t pos = 0;
	size_t pos2 = 0;
	line = remove_end(line, ';');
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	while (pos != std::string::npos) {
		pos2 = line.find_first_of(" \r\t\b\f", pos);
		temp = line.substr(pos, pos2 - pos);
		this->_path.push_back(temp);
		pos = line.find_first_not_of(" \r\t\b\f", pos2);
	}
	std::cout << "done cleaning path" << std::endl;
	if (this->_path.empty())
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

void	configCGI::addToMap()
{
	std::vector<std::string>::iterator it1 = this->_ext.begin();
	std::vector<std::string>::iterator it2 = this->_path.begin();
	while (it1 != this->_ext.end() && it2 != this->_path.end())
	{
		this->_intr_paths.insert(std::map<std::string, std::string>::value_type(*it1, *it2));
		std::cout << *it1 << " " << *it2 << std::endl;
		++it1;
		++it2;
	}
	if (it1 != this->_ext.end() || it2 != this->_path.end())
		this->set_error_code(12);
}
