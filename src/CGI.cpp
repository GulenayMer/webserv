# include "../include/CGI.hpp"

std::map<int, int> exit_status;

CGI::CGI(Response &response): _response(response)
{
	this->_done_reading = false;
	this->_body_complete = false;
	this->_header_removed = false;
	this->_vector_pos = 0;
	this->_bytes_sent = 0;
	for (int i = 0; i < 18; i++)
		this->_exec_env[i] = NULL;
	this->_pid = 0;
	this->env_init();
	this->set_boundary();
}

CGI::CGI(const CGI& obj): _response(obj._response)
{
	*this = obj;
}

CGI& CGI::operator=(const CGI& obj)
{
	if (this != &obj) {
		this->_done_reading = obj._done_reading;
		this->_body_complete = obj._body_complete;
		this->_header_removed = obj._header_removed;
		this->_content_length = obj._content_length;
		this->_vector_pos = obj._vector_pos;
		this->_bytes_sent = obj._bytes_sent;
		for (int i = 0; i < 18; i++)
			this->_exec_env[i] = obj._exec_env[i];
		this->_pid = obj._pid;
		this->_env = obj._env;
		this->_boundary = obj._boundary;
		this->_input_pipe[0] = obj._input_pipe[0];
		this->_input_pipe[1] = obj._input_pipe[1];
		this->_output_pipe[0] = obj._output_pipe[0];
		this->_output_pipe[1] = obj._output_pipe[1];
	}
	return *this;
}

CGI::~CGI()
{
	for (int i = 0; this->_exec_env[i]; i++)
	{
		if (this->_exec_env[i])
			free(this->_exec_env[i]);
	}
}

void	CGI::env_init()
{		
	_env["GATEWAY_INTERFACE"] = std::string("CGI/1.1"); // The revision of the Common Gateway Interface that the server uses.
	_env["SERVER_NAME"] = _response.getConfig().get_server_name(); //  The server's hostname or IP address.
	_env["SERVER_SOFTWARE"] = std::string("webserv"); //  The name and version of the server software that is answering the client request.
	_env["SERVER_PROTOCOL"] = std::string("HTTP/1.1");//  The name and revision of the information protocol the request came in with.
	_env["SERVER_PORT"] = to_string(_response.getConfig().get_port()); //  The port number of the host on which the server is running.
	switch (_response.getRequest().getMethod()) { //  The method with which the information request was issued.
		case POST:
			_env["REQUEST_METHOD"] = "POST";
			break;
		case GET:
			_env["REQUEST_METHOD"] = "GET";
			break;
		default:
			_env["REQUEST_METHOD"] = "BLAH";
	}
	_env["PATH_INFO"] = get_path_from_map(); //Extra path information passed to a CGI program.
	if (_env["PATH_INFO"].length() == 0)
		_env["PATH_TRANSLATED"] = this->_response.getConfig().get_cgi().get_root(); // The translated version of the path given by the variable PATH_INFO.
	else
		_env["PATH_TRANSLATED"] = _env["PATH_INFO"];
	_env["SCRIPT_NAME"] = remove_end(_response.getRequest().getUri(), '?'); // The virtual path (e.g., /cgi-bin/program.pl) of the script being executed.
	//TODO find which location to do, using servers for now.
	_env["DOCUMENT_ROOT"] = this->_response.getConfig().get_root(); // The directory from which Web documents are served.
	_env["QUERY_STRING"] = this->get_query(); // The query information passed to the program. It is appended to the URL with a "?".
	_env["REMOTE_HOST"] = _response.getRequest().get_single_header("Host"); // The remote hostname of the user making the request.
	_env["REMOTE_ADDR"] = std::string("127.0.0.1");//_response.getRequest().get_single_header("Host"); // The remote IP address of the user making the request.
	_env["CONTENT_TYPE"] = this->_response.getRequest().get_single_header("Content-Type"); // The MIME type of the query data, such as "text/html".
	this->_content_length = atol(_response.getRequest().get_single_header("Content-Length").c_str());
	this->_content_length += this->_response.getRequest().getHeaderLength();
	_env["CONTENT_LENGTH"] = to_string(this->_content_length); // The length of the data (in bytes or the number of characters) passed to the CGI program through standard input.
	//_env["HTTP_ACCEPT"]; // A list of the MIME types that the client can accept.
	_env["HTTP_USER_AGENT"] = _response.getRequest().get_single_header("User-Agent");; // The browser the client is using to issue the request.
	_env["HTTP_REFERER"] = _response.getRequest().get_single_header("Referer"); // The URL of the document that the client points to before accessing the CGI program. */
}


void	CGI::env_to_char(void)
{
	std::string temp;
	std::map<std::string, std::string>::iterator it = this->_env.begin();
	int i = 0;
	while (it != this->_env.end()) {
		temp = it->first + "=" + it->second;
		this->_exec_env[i] = strdup(temp.c_str());
		it++;
		i++;	
	}
	this->_exec_env[i] = NULL;
}

int		CGI::handle_cgi()
{
    std::ifstream file;
	std::string new_path = this->_response.getRequest().getUri();
	std::string shebang;

	new_path = remove_end(this->_response.getRequest().getUri(), '?');
   	new_path = "." + new_path;
	// TODO check if ext is allowed
	//std::cout << new_path << std::endl;
	file.open(new_path.c_str(), std::ios::in);
	if (file.fail() == true) {
		close(this->_output_pipe[0]);
		close(this->_output_pipe[1]);
		return -1;
	}
	getline(file, shebang);
	// TODO invalid file, no shebang
	if (shebang.find("#!") == std::string::npos)
	{
		close(this->_output_pipe[0]);
		close(this->_output_pipe[1]);
		file.close();
		return -1;
	}
	size_t pos = shebang.find_last_of("/");
	shebang = &shebang[pos] + 1;
	file.close();
	this->env_to_char();
	this->_pid = fork();
    if (this->_pid == 0)
        exec_script(this->_input_pipe, this->_output_pipe, new_path);
    else
	{
		close(this->_input_pipe[0]);
		close(this->_output_pipe[1]);
	}
	return this->_output_pipe[0];
}

void	CGI::exec_script(int *input_pipe, int *output_pipe, std::string path)
{
	char *args[2];
	close(output_pipe[0]);
	close(input_pipe[1]);
    args[0] = strdup(path.c_str());
	args[1] = NULL;
	dup2(output_pipe[1], STDOUT_FILENO);
	close(output_pipe[1]);
	dup2(input_pipe[0], STDIN_FILENO);
	close(input_pipe[0]);
    execve(args[0], args, this->_exec_env);
    perror("execve failed.");
	exit(1);
}

std::string CGI::get_path_from_map()
{
	std::string ext = remove_end(_response.getRequest().getUri(), '?');
	int pos = ext.find_last_of(".");
	ext = &ext[pos] + 1;
	std::map<std::string, std::string> paths_map = this->_response.getConfig().get_cgi().get_path();
	std::map<std::string, std::string>::iterator it = paths_map.begin();
	std::map<std::string, std::string>::iterator end = paths_map.end();
	std::string path = "";
	while (it != end) {
		if (it->first.find(ext) != std::string::npos) {
			path = it->second;
			break;
		}
		std::advance(it, 1);
	}
	return path;
}

std::string CGI::get_query()
{
	std::string query = this->_response.getRequest().getUri();
	if (query.find("?") != std::string::npos) {
		int pos = query.find("?");
		query = &query[pos] + 1;
	}
	else
		query = "";
	return query;
}
int	CGI::initOutputPipe()
{
    if (pipe(this->_output_pipe) < 0)
    {
        std::cout << "Error opening pipe" << std::endl;
        return -1;
    }
	if (fcntl(this->_output_pipe[0], F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl set_flags");
		close(this->_output_pipe[0]);
		close(this->_output_pipe[1]);
		return -1;
	}
	return this->_output_pipe[0];
}

int	CGI::initInputPipe()
{
	if (pipe(this->_input_pipe) < 0)
	{
		std::cout << "Error opening pipe" << std::endl;
		close(this->_output_pipe[0]);
		close(this->_output_pipe[1]);
		return -1;
	}
	if (fcntl(this->_input_pipe[1], F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl set_flags");
		close(this->_output_pipe[0]);
		close(this->_output_pipe[1]);
		close(this->_input_pipe[0]);
		close(this->_input_pipe[1]);
		return -1;
	}
	return this->_input_pipe[1];
}

void	CGI::sendResponse()
{
	size_t	sent;
	size_t	content_index = 0;
	std::ostringstream response_stream;
	std::string response_string;
	std::string content;
	std::cout << "HERE" << std::endl;
	if (exit_status.find(this->_pid)->second != 0)
	{
		//TODO send 500 -> Internal Server Error
		response_string = this->getResponse().createError(500);
		std::cout << "ERROR" << std::endl;
		sent = send(this->_response.getConnFd(), &response_string[0], response_string.size(), MSG_DONTWAIT);
		std::cout << response_string << std::endl;
	}
	else
	{
		for (; this->_response_buff[content_index] != '\n'; content_index++)
			content += this->_response_buff[content_index];
		for (size_t i = content_index + 2; i > content_index; content_index++)
			content += this->_response_buff[content_index];
		std::cout << content << std::endl;
		if (content.find("Content-Type") == std::string::npos && content.find("content-type") == std::string::npos)
		{
			//TODO send error content type unknown -> internal server error probably
			response_string = this->getResponse().createError(500);
			std::cout << "ERROR" << std::endl;
			sent = send(this->_response.getConnFd(), &response_string[0], response_string.size(), MSG_DONTWAIT);
		}
		else
		{
			std::cout << RED << "Sending response..." << RESET << std::endl;
			response_stream << HTTP_404 << "Content-Length: " << _response_buff.size() - content.size() << "\n" << "Connection: Keep-Alive\n";
			response_string = response_stream.str();
			for (ssize_t i = response_string.size() - 1; i >= 0; i--)
				this->_response_buff.insert(this->_response_buff.begin(), response_string[i]);
			for (size_t i = 0; i < _response_buff.size(); i++)
				std::cout << BLUE << _response_buff[i];
			std::cout << RESET << std::endl;
			sent = send(this->_response.getConnFd(), &_response_buff[0], _response_buff.size(), MSG_DONTWAIT);
		}
	}
	if (sent > 0)
	{
		exit_status.erase(this->_pid);
		// _bytes_sent += sent;
		// _buffer.erase(0, sent);
		// std::cout << RED << "\n\n\n\n" << _buffer << RESET << std::endl;
		// std::cout << RED << "-- BUFFER END --" << RESET << std::endl;
		// if (_bytes_sent == _buffer.length())
		// {
		// 	std::cout << "NOT HERE!!!!" << std::endl;
		// 	_buffer.clear();
		// 	_bytes_sent = 0;
		// }
	}
}

void	CGI::add_to_buffer(char *buff, size_t rec)
{
	for (size_t i = 0; i < rec; i++)
		this->_response_buff.push_back(buff[i]);
}

Response &CGI::getResponse()
{
	return this->_response;
}

std::string& CGI::get_boundary()
{
	return this->_boundary;
}

void	CGI::setReadComplete()
{
	this->_done_reading = true;
}

bool	CGI::readComplete()
{
	return this->_done_reading;
}

bool	CGI::bodyComplete()
{
	return this->_body_complete;
}

void	CGI::set_boundary()
{
	std::cout << _env["CONTENT_TYPE"] << std::endl;
	size_t pos = _env["CONTENT_TYPE"].find("boundary=");
	if (pos != std::string::npos) {
		pos += 9;
		this->_boundary = "--" + _env["CONTENT_TYPE"].substr(pos);
	}
}

void	CGI::storeBuffer(char *buffer, size_t received)
{
	for (size_t i = 0; i < received; i++)
		this->_request_buff.push_back(buffer[i]);
}

int		CGI::getOutFd()
{
	return this->_output_pipe[0];
}

int		CGI::getInFd()
{
	return this->_input_pipe[1];
}

void	CGI::writeToCGI()
{
	if (this->_request_buff.empty())
	{
		this->_vector_pos = 0;
		return;
	}
	size_t sent = write(this->_input_pipe[1], &this->_request_buff[this->_vector_pos], this->_request_buff.size());
	if (sent > 0)
	{
		// for (size_t i = this->_vector_pos; i < sent + this->_vector_pos; i++)
		// 	std::cout << this->_request_buff[i];
		// std::cout << std::endl;
		this->_vector_pos += sent;
		if (this->_vector_pos >= this->_request_buff.size())
		{
			this->_vector_pos = 0;
			this->_request_buff.clear();
		}
		this->_bytes_sent += sent;
		std::cout << "total bytes sent: " << this->_bytes_sent << ", content_length: " << this->_content_length << std::endl;
		if (this->_bytes_sent >= this->_content_length && this->_request_buff.empty())
		{
			std::cout << "DONE" << std::endl;
			this->_body_complete = true;
		}
	}
}
