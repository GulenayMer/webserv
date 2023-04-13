# include "../include/CGI.hpp"

std::map<int, int> exit_status;

CGI::CGI(Response &response): _response(response)
{
	this->_done_reading = false;
	this->_body_complete = false;
	this->_header_removed = false;
	this->_vector_pos = 0;
	this->_bytes_sent = 0;
	this->_errno = 0;
	for (int i = 0; i < 18; i++)
		this->_exec_env[i] = NULL;
	this->_pid = 0;
	this->env_init();
	this->set_boundary();
	this->_header_length = 0;
	this->_chunk_context = false;
	this->_chunk_size = 0;
	this->_chunk_remaining = 0;
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
		this->_errno = obj._errno;
		for (int i = 0; i < 18; i++)
		{
			if (obj._exec_env[i])
				this->_exec_env[i] = strdup(obj._exec_env[i]);
			else
				this->_exec_env[i] = NULL;
		}
		this->_pid = obj._pid;
		this->_env = obj._env;
		this->_boundary = obj._boundary;
		this->_input_pipe[0] = obj._input_pipe[0];
		this->_input_pipe[1] = obj._input_pipe[1];
		this->_output_pipe[0] = obj._output_pipe[0];
		this->_output_pipe[1] = obj._output_pipe[1];
		this->_header_length = obj._header_length;
		this->_chunk_context = obj._chunk_context;
		this->_chunk_size = obj._chunk_size;
		this->_chunk_remaining = obj._chunk_remaining;
	}
	return *this;
}

CGI::~CGI()
{
	for (int i = 0; this->_exec_env[i]; i++)
		free(this->_exec_env[i]);
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
	// _env["PATH_INFO"] = get_path_from_map(); //Extra path information passed to a CGI program.
	// if (_env["PATH_INFO"].length() == 0)
	// 	_env["PATH_TRANSLATED"] = this->_response.getRequest().; // The translated version of the path given by the variable PATH_INFO.
	// else
	// 	_env["PATH_TRANSLATED"] = _env["PATH_INFO"];
	_env["SCRIPT_NAME"] = remove_end(_response.getRequest().getUri(), '?'); // The virtual path (e.g., /cgi-bin/program.pl) of the script being executed.
	//TODO find which location to do, using servers for now.
	_env["DOCUMENT_ROOT"] = this->_response.getConfig().get_root(); // The directory from which Web documents are served.
	_env["QUERY_STRING"] = this->get_query(); // The query information passed to the program. It is appended to the URL with a "?".
	// TODO Host might need to be converted
	if (_response.getRequest().get_single_header("referer").size() > 0)
		_env["REMOTE_HOST"] = _response.getRequest().get_single_header("referer"); // The remote hostname of the user making the request.
	else
		_env["REMOTE_HOST"] = _response.getRequest().get_single_header("host");
	_env["REMOTE_ADDR"] = _response.getRequest().get_single_header("host");//_response.getRequest().get_single_header("Host"); // The remote IP address of the user making the request.
	_env["CONTENT_TYPE"] = this->_response.getRequest().get_single_header("content-type"); // The MIME type of the query data, such as "text/html".
	this->_content_length = atol(_response.getRequest().get_single_header("content-length").c_str());
	this->_content_length += this->_response.getRequest().getHeaderLength();
	_env["CONTENT_LENGTH"] = to_string(this->_content_length); // The length of the data (in bytes or the number of characters) passed to the CGI program through standard input.
	//_env["HTTP_ACCEPT"]; // A list of the MIME types that the client can accept.
	_env["HTTP_USER_AGENT"] = _response.getRequest().get_single_header("user-agent");; // The browser the client is using to issue the request.
	_env["HTTP_REFERER"] = _response.getRequest().get_single_header("referer"); // The URL of the document that the client points to before accessing the CGI program. */
	if (_response.getRequest().get_single_header("cookie").size() > 0)
		_env["HTTP_COOKIE"] = _response.getRequest().get_single_header("cookie");
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

bool	CGI::handle_cgi()
{
    std::ifstream file;
	std::string script_path = this->_response.getRequest().getUri();
	std::string shebang;

	std::cout << "CGI script path: " << script_path << std::endl;
	std::map<std::string, std::string>::const_iterator path_it = this->_response.getConfig().getIntrPath().find(this->_response.getExt());
	if (path_it == this->_response.getConfig().getIntrPath().end())
	{
		std::cout << "CGI Script interpreter path not found for:" << this->_response.getExt() << std::endl;
		std::cout << "Known interpreters: ";
		for (path_it = this->_response.getConfig().getIntrPath().begin(); path_it != this->_response.getConfig().getIntrPath().end(); path_it++)
			std::cout << path_it->second << " ";
		std::cout << std::endl;
		this->_errno = 1;
		return false;
	}
	// file.open(new_path.c_str(), std::ios::in);
	// if (file.fail() == true) {
	// 	std::cout << "CGI FILE OPEN FAIL: " << new_path << std::endl;
	// 	this->_errno = 1;
	// 	return false;
	// }
	// getline(file, shebang);
	// // TODO invalid file, no shebang
	// if (shebang.find("#!") == std::string::npos)
	// {
	// 	std::cout << "CGI NO SHEBANG FAIL: " << std::endl;
	// 	this->_errno = 2;
	// 	file.close();
	// 	return false;
	// }
	// size_t pos = shebang.find_last_of("/");
	// shebang = &shebang[pos] + 1;
	// file.close();
	this->_pid = fork();
    if (this->_pid == 0)
	{
		this->env_to_char();
		std::cout << "CGI script path: " << script_path << std::endl;
		char cwd[PATH_MAX];
		if (getcwd(cwd, sizeof(cwd)) != NULL) {
			std::cout << "Current working dir: " << cwd << std::endl;
		} else {
			perror("getcwd() error");
		}
        exec_script(this->_input_pipe, this->_output_pipe, script_path);
	}
    else
	{
		if (this->_input_pipe[0] > 0)
			close(this->_input_pipe[0]);
		this->_input_pipe[0] = -1;
		if (this->_output_pipe[1] > 0)
			close(this->_output_pipe[1]);
		this->_output_pipe[1] = -1;
	}
	return true;
}

void	CGI::exec_script(int *input_pipe, int *output_pipe, std::string path)
{
	char *args[2];
	if (output_pipe[0] > 0)
		close(output_pipe[0]);
	if (input_pipe[1] > 0)
		close(input_pipe[1]);
    args[0] = strdup(path.c_str());
	args[1] = NULL;
	dup2(output_pipe[1], STDOUT_FILENO);
	if (output_pipe[1] > 0)
		close(output_pipe[1]);
	dup2(input_pipe[0], STDIN_FILENO);
	if (input_pipe[0] > 0)
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
	std::map<std::string, std::string> paths_map = this->_response.getConfig().getIntrPath();
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
		if (this->_output_pipe[0] > 0)
			close(this->_output_pipe[0]);
		this->_output_pipe[0] = -1;
		if (this->_output_pipe[1] > 0)
			close(this->_output_pipe[1]);
		this->_output_pipe[1] = -1;
		return -1;
	}
	return this->_output_pipe[0];
}

int	CGI::initInputPipe()
{
	if (pipe(this->_input_pipe) < 0)
	{
		std::cout << "Error opening pipe" << std::endl;
		if (this->_output_pipe[0] > 0)
			close(this->_output_pipe[0]);
		this->_output_pipe[0] = -1;
		if (this->_output_pipe[1] > 0)
			close(this->_output_pipe[1]);
		this->_output_pipe[1] = -1;
		return -1;
	}
	if (fcntl(this->_input_pipe[1], F_SETFL, O_NONBLOCK) == -1)
	{
		perror("fcntl set_flags");
		if (this->_output_pipe[0] > 0)
			close(this->_output_pipe[0]);
		this->_output_pipe[0] = -1;
		if (this->_output_pipe[1] > 0)
			close(this->_output_pipe[1]);
		this->_output_pipe[1] = -1;
		if (this->_input_pipe[0] > 0)
			close(this->_input_pipe[0]);
		this->_input_pipe[0] = -1;
		if (this->_input_pipe[1] > 0)
			close(this->_input_pipe[1]);
		this->_input_pipe[1] = -1;
		return -1;
	}
	return this->_input_pipe[1];
}

bool	CGI::sendResponse()
{
	ssize_t	sent;
	std::ostringstream response_stream;
	std::string response_string;
	std::string content;
	if (exit_status.find(this->_pid)->second != 0 || this->_errno != 0)
	{
		response_string = this->getResponse().createError(500, &this->getResponse().getConfig());
		std::cout << "ERROR 505 sending" << std::endl;
		_content_length = response_string.size();
		sent = send(this->_response.getConnFd(), &response_string[0], response_string.size(), MSG_DONTWAIT);
		std::cout << response_string << std::endl;
	}
	else if (_content_length == 0)
	{
		response_string = "HTTP/1.1 204 OK\r\nConnection: Keep-Alive\r\n\r\n";
		std::cout << "CGI complete - nothing to send" << std::endl;
		_content_length = response_string.size();
		sent = send(this->_response.getConnFd(), &response_string[0], response_string.size(), MSG_DONTWAIT);
		std::cout << response_string << std::endl;
	}
	else if (this->_response.getExt() == ".php")
	{
		std::cout << RED << "Sending php response..." << RESET << std::endl;
		if (this->_bytes_sent == 0)
		{
			for (int i = 0; i < 9; i++)
			{
				if (this->_response_buff[i] != "HTTP/1.1"[i])
				{
					std::stringstream ss;
					ss << _response_buff.size();
					std::vector<char>::iterator it = this->_response_buff.begin();
					std::string size(ss.str());
					for (int j = 27; j >= 0; j--)
						this->_response_buff.insert(it, "\nContent-Type:text/html\r\n\r\n"[j]);
					for (int j = size.size() - 1; j >= 0; j--)
						this->_response_buff.insert(it, size[j]);
					for (int j = 32; j >= 0; j--)
						this->_response_buff.insert(it, "HTTP/1.1 200 OK\r\nContent-Length:"[j]);
					break;
				}
			}
			_content_length = this->_response_buff.size();
		}
		for (size_t i = 0; i < _response_buff.size(); i++)
			std::cout << BLUE << _response_buff[i];
		std::cout << RESET << std::endl;
		sent = send(this->_response.getConnFd(), &_response_buff[this->_bytes_sent], _response_buff.size() - this->_bytes_sent, MSG_DONTWAIT);
	}
	else
	{
		std::cout << RED << "Sending response..." << RESET << std::endl;
		for (size_t i = 0; i < _response_buff.size(); i++)
			std::cout << BLUE << _response_buff[i];
		std::cout << RESET << std::endl;
		sent = send(this->_response.getConnFd(), &_response_buff[this->_bytes_sent], _response_buff.size() - this->_bytes_sent, MSG_DONTWAIT);
	}
	std::cout << "sent: " << _bytes_sent << ", content length: " << _content_length << std::endl;
	if (sent >= 0)
	{
		this->_bytes_sent += sent;
		if (this->_bytes_sent == this->_content_length)
		{
			std::cout << "SEND COMPLETE" << std::endl;
			exit_status.erase(this->_pid);
			_response_buff.clear();
			this->_done_reading = false;
			return true;
		}
	}
	return false;
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
	this->_content_length = this->_response_buff.size();
	this->_done_reading = true;
}

bool	CGI::readComplete()
{
	return this->_done_reading;
}

bool	CGI::bodySentCGI()
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
	std::cout << "STORE BUFFER" << std::endl;
	for (size_t i = 0; i < received; i++)
		this->_request_buff.push_back(buffer[i]);
	std::cout << "STORE BUFFER END" << std::endl;
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
	ssize_t sent = write(this->_input_pipe[1], &this->_request_buff[this->_vector_pos], this->_request_buff.size() - this->_vector_pos);
	if (sent > 0)
	{
		this->_vector_pos += sent;
		if (this->_vector_pos >= this->_request_buff.size())
		{
			this->_vector_pos = 0;
			this->_request_buff.clear();
		}
		this->_bytes_sent += sent;
		std::cout << "total bytes sent: " << this->_bytes_sent << ", content_length: " << this->_content_length << std::endl;
		if (this->_bytes_sent >= this->_content_length)
		{
			std::cout << "DONE" << std::endl;
			this->_body_complete = true;
			this->_bytes_sent = 0;
			this->_content_length = 0;
		}
	}
}

bool CGI::completeContent()
{
	if (this->_request_buff.size() - this->_content_length == 0)
		return true;
	return false;
}

int	CGI::PID()
{
	return this->_pid;
}

void	CGI::closePipes()
{
	if (this->_input_pipe[0] > 0)
		close(this->_input_pipe[0]);
	if (this->_input_pipe[1] > 0)
		close(this->_input_pipe[1]);
	if (this->_output_pipe[0] > 0)
		close(this->_output_pipe[0]);
	if (this->_output_pipe[1] > 0)
		close(this->_output_pipe[1]);
}

void CGI::mergeChunk(char *buffer, size_t received) //TODO need to check if \r\n removed from output
{
	size_t	pos = 0;
	while (pos < received)
	{
		if (!this->_chunk_context)
		{
			pos = convertHex(&buffer[pos]);
			this->_chunk_remaining = this->_chunk_size;
			if (this->_chunk_size == 0)
			{
				addHeaderChunked();
				this->_content_length += this->_header_length;
				this->_response.finishChunk();
				std::cout << "Complete message:" << std::endl;
				for (size_t i = 0; i < this->_request_buff.size(); i++)
					std::cout << this->_request_buff[i];
				std::cout << std::endl;
				std::cout << "Message end." << std::endl;
				return;
			}
		}
		if (this->_chunk_remaining >= received - pos)
		{
			storeBuffer(&buffer[pos], received - pos);
			this->_chunk_remaining -= (received - pos);
			pos = received;
		}
		else
		{
			storeBuffer(&buffer[pos], this->_chunk_remaining);
			pos += this->_chunk_remaining + 2;
			this->_chunk_context = false;
			this->_chunk_remaining = 0;
		}
	}
}

size_t CGI::convertHex(char *buffer)
{
	// Convert hexadecimal number to decimal
	char *stopstr;
	int	i = 0;

	// Find the end of the hex number
	while (buffer[i] && buffer[i] != '\r' && buffer[i] != '\n')
		i++;
	// TODO: check if this is correct
	// if (i == 0)
	// 	throw std::runtime_error("CGI::convertHex: No hexadecimal number found");
	this->_chunk_size = std::strtoul(buffer, &stopstr, 16);
	// TODO: check if this is correct
	// if (this->_chunk_size == 0 && std::strcmp(buffer, "0"))
	// 	throw std::runtime_error("CGI::convertHex: Invalid hexadecimal number");
	this->_content_length += this->_chunk_size;
	i += 2;
	this->_chunk_context = true;
	return i;
}

void CGI::addHeaderChunked()
{
	std::map<std::string, std::string>::const_iterator it = this->_response.getRequest().getCompleteHeader().begin();
	std::ostringstream oss;
	oss << this->_content_length;
	std::string len(oss.str() + "\r\n\r\n");
	for (int i = len.length() - 1; i > 0; i--)
	{
		this->_request_buff.insert(this->_request_buff.begin(), len[i]);
		this->_header_length++;
	}
	for (int i = 16; i >= 0; i--)
	{
		this->_request_buff.insert(this->_request_buff.begin(), "Content-Length: "[i]);
		this->_header_length++;
	}
	for (; it != this->_response.getRequest().getCompleteHeader().end(); it++)
	{
		if (it->first == "transfer-encoding")
			continue;
		this->_request_buff.insert(this->_request_buff.begin(), '\n');
		this->_request_buff.insert(this->_request_buff.begin(), it->second.begin(), it->second.end());
		this->_request_buff.insert(this->_request_buff.begin(), ' ');
		this->_request_buff.insert(this->_request_buff.begin(), ':');
		this->_request_buff.insert(this->_request_buff.begin(), it->first.begin(), it->first.end());
		this->_header_length += it->second.length() + it->first.length() + 2;
	}
}

void	CGI::removeHeader(char *buffer, ssize_t received)
{
	for (ssize_t i = 0; i < received; i++)
	{
		if (buffer[i] == '\r' && buffer[i + 1] == '\n')
		{
			if (buffer[i + 2] == '\r' && buffer[i + 3] == '\n')
			{
				mergeChunk(&buffer[i + 4], received - (i + 4));
				break;
			}
		}
	}
}
