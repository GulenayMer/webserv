#ifndef UTILS_HPP
# define UTILS_HPP


#include <iostream> 
#include <string>
#include <cstring>
#include <map>
#include <iterator>
#include <csignal> // for signals
#include <cstdlib> // for dynamic mem. management
#include <fstream> // streams for file operations
#include <sstream> // constructs stringstream obj.s
#include <sys/types.h>
#include <sys/wait.h> // waitpid
#include <poll.h> // definitions for the poll func.
#include <errno.h>
#include <arpa/inet.h> // for internet operations & struct in_addr
#include <sys/socket.h> // for socket()
#include <unistd.h> // for access(), execve() 
#include "httpHeader.hpp"
#include "minilib.hpp" //our functions


const int MAX_CONN = 10000;
const int BUFFER_SIZE = 2048;
extern int SWITCH;

/*

	Colors

*/

# define RESET   "\033[0m"
# define BLACK   "\033[30m"
# define RED     "\033[31m"
# define GREEN   "\033[32m"
# define YELLOW  "\033[33m"
# define BLUE    "\033[34m"
# define PURPLE "\033[91m"
# define CYAN    "\033[36m"
# define WHITE   "\033[37m"

/*

	Config keywords

*/

# define LISTEN 			"listen"
# define HOST 				"host"
# define SERVER_NAME 		"server_name"
# define ERROR_PAGE			"error_page"
# define ALLOW_METHODS 		"allow_methods"
# define CLIENT_MAX_BODY	"client_max_body_size"									  
# define ROOT 				"root"
# define INDEX 				"index"
# define ALIAS 				"alias"
# define ALLOW_CGI 			"allow_cgi"
# define AUTOINDEX 			"autoindex"
# define RETURN 			"return"
# define CGI_EXT 			"cgi_ext"
# define CGI_PATH 			"cgi_path"
# define REDIR    			"redirection"
# define UPLOAD_STORE 		"upload_store"


/*

	Error Messages

*/

// Too many arguments
# define TOO_MANY_ARGS "ERROR: --- Too many arguments ---\nCorrect use: ./webserv [optional config]"

// Error in 404
# define	_404_ERROR "error opening 404 file\n"

// Error in response sending
# define _RES_ERROR "error while sending the response"

// error_code 1
# define NO_DEFAULT_CONFIG "ERROR: --- Could not find default configuration file at: ./webserver.config ---"

// error_code 2
# define NO_VALID_SERVER "ERROR: --- Could not find a valid ** server {} ** context in the provided configuration file ---"

// error_code 3
# define NO_VALID_PORT "ERROR: --- Could not find a valid ** port ** configuration ---"

// error_code 4
# define NO_VALID_HOST "ERROR: --- Could not find a valid ** host ** configuration ---"

// error_code 5
# define NO_VALID_ERROR_PAGE "ERROR: --- Could not find a valid ** error_page ** configuration ---"

// error_code 6
# define NO_VALID_SERVER_NAME "ERROR: --- An empty ** server_name ** was provided ---"

// error_code 7
# define NO_VALID_CLIENT_MAX_BODY_SIZE "ERROR: --- Could not find a valid ** client_max_body_size ** configuration ---"

// error_code 8
# define NO_VALID_AUTOINDEX "ERROR: --- Could not find a valid ** autoindex ** configuration ---"

// error_code 9
# define NO_VALID_ROOT "ERROR: --- Could not find a valid ** root ** configuration ---"

// error_code 10
# define NO_VALID_INDEX "ERROR: --- Could not find a valid ** index ** configuration ---"

// error_code 11
# define NO_VALID_LOCATION "ERROR: --- An invalid ** location ** context was provided ---"

// error_code 12
# define NO_VALID_CGI "ERROR: --- An invalid ** CGI ** context was provided ---"

// error_code 13
# define NO_VALID_METHODS "ERROR: --- Could not find a valid ** allowed_methods ** configuration ---"

// error_code 14
# define NO_VALID_CGI_PATH "ERROR: --- Could not find a valid ** cgi_path ** configuration ---"

// error_code 15
# define NO_VALID_CGI_EXT "ERROR: --- Could not find a valid ** cgi_ext ** configuration ---"

// error_code 16
# define SOCK_ERROR "ERROR: --- Could not create socket ---"

// error_code 17
# define SOCK_OPT_ERROR "ERROR: --- Could not set socket options ---"

// error_code 18
# define BIND_ERROR "ERROR: --- Could not bind socket ---"

// error_code 19
# define LISTEN_ERROR "ERROR: --- Could not listen on socket fd ---"

// error_code 20
# define INVALID_ROOT "ERROR: --- Provided  ** root ** does not exist ---"

// error_code 21
# define INVALID_INDEX "ERROR: --- Provided  ** index ** does not exist ---"

// error_code 22
# define INVALID_ERROR_CHECK "ERROR: --- One of the provided ** default_error ** pages does not exist ---"

// error_code 23
# define INVALID_LOCATION_ROOT "ERROR: --- One of the provided locations ** root ** does not exist ---"

// error_code 24
# define INVALID_LOCATION_INDEX "ERROR: --- One of the provided locations ** index ** does not exist ---"

// error_code 25
# define INVALID_LOCATION_REDIRECTION "ERROR: --- One of the provided locations ** redirection ** does not exist ---"

// error_code 26
# define INVALID_METHODS "ERROR: --- One of the provided locations ** methods ** are not valid ---"

// error_code 27
# define INVALID_CGI_ROOT "ERROR: --- Provided  ** cgi-bin root ** does not exist ---"

// error_code 27
# define INVALID_PROGRAM_PATH "ERROR: --- One of your provided cgi program ** path ** does not exist ---"

// error_code 28
# define NO_VALID_CONFIG_FILE "ERROR: --- Could not open provided configuration file ---"

// error_code 29
# define FCNTL_ERROR "ERROR: --- Could not make socket non-blocking ---"

// error_code 30
# define INVALID_UPLOAD_STORE "ERROR: --- Could not find a valid ** upload_store ** in configuration ---"

// error_code 
# define NO_VALID_SERVERS "ERROR: --- No valid configurations provided to create servers ---"

/* HTML response */

#define HTTP_OK "HTTP/1.1 200 OK\r\n"
#define HTTP_204 "HTTP/1.1 204 No Content\r\n"
#define HTTP_404 "HTTP/1.1 404 Not Found\r\n"
#define HTTP_500 "HTTP/1.1 500 Internal Server Error\r\n"

#endif