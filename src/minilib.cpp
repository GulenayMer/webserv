#include "../include/minilib.hpp"
#include "../include/Utils.hpp"
#include <sstream>
#include <sys/types.h>
#include <dirent.h>
#include <iomanip>

int	SWITCH = 1;
extern std::map<int, int> exit_status;

std::string readFile(std::string filename)
{
	std::ifstream file(filename.c_str());
	if (!file.is_open())
		return (NULL);
	std::stringstream file_buffer;
	file_buffer << file.rdbuf();

	return (file_buffer.str());
}

bool		dir_exists(std::string path)
{
	DIR* dir = opendir(path.c_str());
	if (dir) {
		closedir(dir);
		return true;
	}
	else {
		return false;
	}
	
}

bool		file_exists(std::string file)
{
	std::ifstream in_file;
	
	in_file.open(file.c_str(), std::ios::in);
	if (in_file.fail() == true) {
		return false;
	}
	in_file.close();
	return true;
}

bool	check_def_format(std::string def, std::string line)
{
	size_t pos = line.find(def);
	if ((pos == 0 || pos == line.find_first_not_of(" \r\t\b\f")) 
		&& line.find_first_of(" \r\t\b\f", pos) - pos == def.size())
		return true;
	return false;
}

std::string remove_end(std::string line, char symbol)
{
	if (line.find(symbol) != std::string::npos) {
		line.erase(line.find(symbol));
	}
	return line;
}

std::string remove_comments(std::string line)
{
	line = remove_end(line, '#');
	return line;
}

std::string find_int(std::string line, int loc)
{
	line = get_word(line, loc);
	if (line.empty())
		return line;
	for (size_t i = 0; i < line.length(); i++)
	{
		if (!isdigit(line[i]))
		{
			line.clear();
			return line;
		}
	}
	return line;
}

std::string	get_word(std::string line, int loc)
{
	size_t	pos;
	line = remove_end(line, ';');
	for (int i = 0; i < loc; i++)
	{	
		pos = line.find_first_not_of(" \r\t\b\f");
		pos = line.find_first_of(" \r\t\b\f", pos);
		pos = line.find_first_not_of(" \r\t\b\f", pos);
		if (pos == std::string::npos)
		{
			line.clear();
			return line;
		}
		line.erase(0, pos);
	}
	pos = line.find_first_of(" \r\t\b\f");
	if (pos != std::string::npos)
		line.erase(pos);
	return line;
}

std::string get_value(std::string line)
{
	line = remove_end(line, ';');	
	size_t pos = line.find_first_not_of(" \r\t\b\f");
	pos = line.find_first_of(" \r\t\b\f", pos);
	pos = line.find_first_not_of(" \r\t\b\f", pos);
	line.erase(0, pos);
	pos = line.find_first_of(" \r\t\b\f");
	if (pos != std::string::npos)
		line.erase(pos);
	return line;
}

int    to_int(std::string str)
{
    int    x;
    std::istringstream ss(str);
    ss >> x;
    return x;
}

std::string to_string(int num)
{
	std::stringstream ss;
	ss << num;
	std::string str = ss.str();
	return (str);
}

int			get_method_num(std::string method)
{
	if (method == "GET")
		return 0;
	else if (method == "POST")
		return 1;
	else if (method == "DELETE")
		return 2;
	else if (method == "PUT")
		return 3;
	else if (method == "HEAD")
		return 4;
	else if (method == "OPTIONS")
		return 5;
	else if (method == "TRACE")
		return 6;
	else if (method == "CONNECT")
		return 7;
	else if (method == "NONE")
		return 8;
	else
		return -1;
}

void signal_callback_handler(int signum) {
   
   // Terminate program
	(void) signum;
 	SWITCH = 0;
}

void grim_reaper(int signum)
{
	int	status;
	int	pid;

	(void)signum;
	pid = waitpid(-1, &status, WNOHANG);
	while (pid > 0)
	{
		// exit_status.insert(std::map<int, int>::value_type(pid, WEXITSTATUS(status)));
		pid = waitpid(-1, &status, WNOHANG);
	}
}

std::string clean_response_path(std::string response_path)
{
	if (response_path[0] == '/')
		return &response_path[1];
	return response_path;
}

std::string encodeURI(const std::string &uri)
{
  std::ostringstream decoded;
  for (size_t i = 0; i < uri.length(); i++) {
    char c = uri[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      decoded << c;
      continue;
    }
    decoded << std::uppercase << '%' << std::setw(2) << int((unsigned char) c);
    decoded << std::nouppercase;
  }
  return decoded.str();
}

std::string decodeURI(const std::string& input) {
    std::stringstream output;
    for (std::size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '%') {
            if (i + 2 < input.size() && isxdigit(input[i + 1]) && isxdigit(input[i + 2])) {
                int value = 0;
                std::istringstream hex_chars(input.substr(i + 1, 2));
                hex_chars >> std::hex >> value;
                output << static_cast<char>(value);
                i += 2;
            } else {
                output << input[i];
            }
        } else if (input[i] == '+') {
            output << ' ';
        } else {
            output << input[i];
        }
    }
    return output.str();
}

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    for (std::string::iterator it = result.begin(); it != result.end(); ++it) {
        *it = std::tolower(*it);
    }
    return result;
}
