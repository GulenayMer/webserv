# include "../include/CGI.hpp"

CGI::CGI(Config config, httpHeader request, std::string response_body)
{
	this->_config = config;
	this->_request = request;
	this->_response_body = response_body;
}

CGI::CGI(const CGI& obj)
{
	*this = obj;
}

CGI& CGI::operator=(const CGI& obj)
{
	if (this != &obj) {
		this->_request = obj._request;
		this->_response_body = obj._response_body;
		this->_config = obj._config;
	}
	return *this;
}

CGI::~CGI() {}

std::string	CGI::get_response_body() 
{
	return this->_response_body;
}

void	CGI::env_init()
{
	_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1"); // The revision of the Common Gateway Interface that the server uses.
	_env["SERVER_NAME"] = _config.get_server_name(); //  The server's hostname or IP address.
	_env["SERVER_SOFTWARE"] = std::string("webserv"); //  The name and version of the server software that is answering the client request.
	_env["SERVER_PROTOCOL"] = std::string("HTTP/1.1");//  The name and revision of the information protocol the request came in with.
	_env["SERVER_PORT"] = to_string(_config.get_port()); //  The port number of the host on which the server is running.
	_env["REQUEST_METHOD"] = _request.getMethod(); //  The method with which the information request was issued.
	_env["PATH_INFO"]; //Extra path information passed to a CGI program.
	_env["PATH_TRANSLATED"]; // The translated version of the path given by the variable PATH_INFO.
	_env["SCRIPT_NAME"]; // The virtual path (e.g., /cgi-bin/program.pl) of the script being executed.
	_env["DOCUMENT_ROOT"]; // The directory from which Web documents are served.
	_env["QUERY_STRING"]; // The query information passed to the program. It is appended to the URL with a "?".
	_env["REMOTE_HOST"]; // The remote hostname of the user making the request.
	_env["REMOTE_ADDR"]; // The remote IP address of the user making the request.
	_env["AUTH_TYPE"]; // The authentication method used to validate a user.
	_env["REMOTE_USER"]; // The authenticated name of the user.
	_env["REMOTE_IDENT"]; // The user making the request. This variable will only be set if NCSA IdentityCheck flag is enabled, and the client machine supports the RFC 931 identification scheme (ident daemon).
	_env["CONTENT_TYPE"] = _request.get_single_header("content-type"); // The MIME type of the query data, such as "text/html".
	_env["CONTENT_LENGTH"]; // The length of the data (in bytes or the number of characters) passed to the CGI program through standard input.
	//_env["HTTP_FROM"]; // The email address of the user making the request. Most browsers do not support this variable.
	_env["HTTP_ACCEPT"]; // A list of the MIME types that the client can accept.
	_env["HTTP_USER_AGENT"]; // The browser the client is using to issue the request.
	_env["HTTP_REFERER"]; // The URL of the document that the client points to before accessing the CGI program. */
}

int		CGI::handle_cgi()//std::ostringstream &response_stream)
{
    std::ifstream file;
	int fd[2];
	std::string new_path = _request.getUri();
	std::string shebang;
	char buff[1000];
	memset(buff, 0, 1000); // TODO we need to initialize buff (otherwise gives cond.jump etc. error) (std::fill)
    if (pipe(fd) < 0)
    {
        std::cout << "Error opening pipe" << std::endl;
        return EXIT_FAILURE;
    }
	new_path = remove_end(_request.getUri(), '?');
   	new_path = "." + new_path;
	// TODO check if ext is allowed
	std::cout << new_path << std::endl;
	file.open(new_path.c_str(), std::ios::in);
	if (file.fail() == true) {
		close(fd[0]);
		close(fd[1]);
		return EXIT_FAILURE;
	}
	getline(file, shebang);
	// TODO invalid file, no shebang
	if (shebang.find("#!") == std::string::npos)
	{
		close(fd[0]);
		close(fd[1]);
		file.close();
		return EXIT_FAILURE;
	}
	size_t pos = shebang.find_last_of("/");
	shebang = &shebang[pos] + 1;
	file.close();
	pid_t pid = fork();
    if (pid == 0)
        exec_script(fd, new_path, shebang);
    else
    {
		close(fd[1]);
		//waitpid(pid, NULL, 0);
		while (read(fd[0], buff, sizeof(buff) - 1)) {
			_response_body += buff;
			memset(buff, 0, 1000);
		}
		close(fd[0]);
    }
	return EXIT_SUCCESS;
}

void	CGI::exec_script(int *pipe, std::string path, std::string program)
{
    char *args[2];
	close(pipe[0]);
    args[0] = strdup(_config.get_cgi().get_path().find(program.c_str())->second.c_str());
    args[1] = strdup(path.c_str());
    args[2] = NULL;
	dup2(pipe[1], STDOUT_FILENO);
	close(pipe[1]);
    execve(args[0], args, NULL);
    perror("execve failed.");
	exit(0);
}