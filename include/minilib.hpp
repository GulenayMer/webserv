#ifndef MINILIB_HPP
# define MINILIB_HPP

# include <iostream>

std::string readFile(std::string filename);
bool		dir_exists(std::string path);
bool		file_exists(std::string file);
bool		check_def_format(std::string def, std::string line);
std::string remove_comments(std::string line);
std::string remove_end(std::string line, char symbol);
std::string find_int(std::string line, int loc);
std::string get_value(std::string line);
int    		to_int(std::string str);
std::string to_string(int num);
bool 		check_def_format(std::string def, std::string line);
int			get_method_num(std::string method);
void 		signal_callback_handler(int signum);
void		grim_reaper(int signum);
std::string clean_response_path(std::string response_path);
std::string	get_word(std::string line, int loc);
std::string decodeURI(const std::string &uri);
std::string encodeURI(const std::string &uri);
std::string toLowerCase(const std::string& str);

#endif