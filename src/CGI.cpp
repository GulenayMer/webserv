# include "../include/CGI.hpp"

std::map<int, int> exit_status;

/**
 * @brief Construct a new CGI::CGI object
 * 
 * @param response response object required to process CGI
 * @param header header object required to process CGI
 */
CGI::CGI(Response &response, httpHeader &header): _response(response), _header(header)
{
	std::cout << "CGI constructor" << std::endl;
	this->_done_reading = false;
	this->_body_complete = false;
	this->_header_removed = false;
	this->_vector_pos = 0;
	this->_bytes_sent = 0;
	this->_errno = 0;
	std::cout << "zero env" << std::endl;
	for (int i = 0; i < 20; i++)
		this->_exec_env[i] = NULL;
	this->_pid = 0;
	std::cout << "env init" << std::endl;
	this->env_init();
	std::cout << "set boundary" << std::endl;
	this->set_boundary();
	this->_header_length = 0;
	this->_chunk_remaining = 0;
	std::cout << "CGI constructor complete" << std::endl;
}

/**
 * @brief Construct a new CGI::CGI object
 * 
 * @param obj CGI object to be copied
 */
CGI::CGI(const CGI& obj): _response(obj._response)
{
	*this = obj;
}

/**
 * @brief Assinment operator overload
 * 
 * @param obj CGI object to be copied
 * @return CGI& 
 */
CGI& CGI::operator=(const CGI& obj)
{
	std::cout << "CGI assignement operator" << std::endl;
	if (this != &obj) {
		std::cout << "done reading" << std::endl;
		this->_done_reading = obj._done_reading;
		std::cout << "body complete" << std::endl;
		this->_body_complete = obj._body_complete;
		std::cout << "header removed" << std::endl;
		this->_header_removed = obj._header_removed;
		std::cout << "content length" << std::endl;
		this->_content_length = obj._content_length;
		std::cout << "vector pos" << std::endl;
		this->_vector_pos = obj._vector_pos;
		std::cout << "bytes sent" << std::endl;
		this->_bytes_sent = obj._bytes_sent;
		std::cout << "errno" << std::endl;
		this->_errno = obj._errno;
		std::cout << "exec env" << std::endl;
		for (int i = 0; i < 20; i++)
		{
			if (obj._exec_env[i])
				this->_exec_env[i] = strdup(obj._exec_env[i]);
			else
				this->_exec_env[i] = NULL;
		}
		std::cout << "pid" << std::endl;
		this->_pid = obj._pid;
		std::cout << "env" << std::endl;
		this->_env = obj._env;
		std::cout << "boundary" << std::endl;
		this->_boundary = obj._boundary;
		std::cout << "input pipe" << std::endl;
		this->_input_pipe[0] = obj._input_pipe[0];
		this->_input_pipe[1] = obj._input_pipe[1];
		std::cout << "output pipe" << std::endl;
		this->_output_pipe[0] = obj._output_pipe[0];
		this->_output_pipe[1] = obj._output_pipe[1];
		std::cout << "header length" << std::endl;
		this->_header_length = obj._header_length;
		std::cout << "chunk remaining" << std::endl;
		this->_chunk_remaining = obj._chunk_remaining;
		std::cout << "response" << std::endl;
		this->_response = obj._response;
	}
	std::cout << "CGI assignement operator complete" << std::endl;
	return *this;
}

/**
 * @brief Destroy the CGI::CGI object
 * 
 */
CGI::~CGI()
{
	for (int i = 0; this->_exec_env[i]; i++)
		free(this->_exec_env[i]);
}

/**
 * @brief Initializes the enviromental variables to be sent to CGI program
 * 
 */
void	CGI::env_init()
{
	std::cout << "env_init" << std::endl;
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
	_env["HTTP_ACCEPT"] = this->_response.getRequest().get_single_header("accept"); // A list of the MIME types that the client can accept.
	_env["HTTP_USER_AGENT"] = _response.getRequest().get_single_header("user-agent");; // The browser the client is using to issue the request.
	_env["HTTP_REFERER"] = _response.getRequest().get_single_header("referer"); // The URL of the document that the client points to before accessing the CGI program. */
	if (_response.getRequest().get_single_header("cookie").size() > 0)
		_env["HTTP_COOKIE"] = _response.getRequest().get_single_header("cookie");
	_env["UPLOAD_PATH"] = _response.getConfig().get_upload_store();
	_env["DOCUMENT_ROOT"] = _response.getConfig().get_root();
	std::cout << "env_init done" << std::endl;
}

/**
 * @brief Processes the enviromental variables into a format that is
 * passable to execve
 * 
 */
void	CGI::env_to_char(void)
{
	std::string temp;
	std::map<std::string, std::string>::iterator it = this->_env.begin();
	int i = 0;
	while (it != this->_env.end()) {
		temp = it->first + "=" + it->second;
		this->_exec_env[i++] = strdup(temp.c_str());
		it++;
	}
	this->_exec_env[i] = NULL;
}

/**
 * @brief Prepares the request to be sent to CGI
 * 
 * @return true in case of sucess
 * \n 
 * @return false in case of failure
 */
bool	CGI::handle_cgi()
{
    std::ifstream file;
	std::string script_path = this->_response.getRequest().getUri();
	std::string shebang;

	std::map<std::string, std::string>::const_iterator path_it = this->_response.getConfig().getIntrPath().find(this->_response.getExt());
	// if (!file_exists(script_path))
	if (access(script_path.c_str(), F_OK) == -1)
	{
		this->_errno = 1;
		return false;
	}
	else if (path_it == this->_response.getConfig().getIntrPath().end())
	{
		std::cout << "CGI Script interpreter path not found for:" << this->_response.getExt() << std::endl;
		std::cout << "Known interpreters: ";
		for (path_it = this->_response.getConfig().getIntrPath().begin(); path_it != this->_response.getConfig().getIntrPath().end(); path_it++)
			std::cout << path_it->second << " ";
		std::cout << std::endl;
		this->_errno = 2;
		return false;
	}
	this->_pid = fork();
    if (this->_pid == 0)
	{
		this->env_to_char();
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
	std::string script_name;
	size_t pos = path.find_last_of("/");
	if (pos != std::string::npos)
	{
		script_name = path.substr(pos + 1);
		std::string cwd(path.substr(0, path.find_last_of("/")));
		chdir(cwd.c_str());
	}
	else
		script_name = path;
	if (output_pipe[0] > 0)
		close(output_pipe[0]);
	if (input_pipe[1] > 0)
		close(input_pipe[1]);
    args[0] = strdup(script_name.c_str());
	args[1] = NULL;
	dup2(output_pipe[1], STDOUT_FILENO);
	if (output_pipe[1] > 0)
		close(output_pipe[1]);
	dup2(input_pipe[0], STDIN_FILENO);
	if (input_pipe[0] > 0)
		close(input_pipe[0]);
    execve(args[0], args, this->_exec_env);
    perror("execve");
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
		this->_errno = 2;
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
		this->_errno = 2;
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
		this->_errno = 2;
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
		this->_errno = 2;
		return -1;
	}
	return this->_input_pipe[1];
}

bool	CGI::sendResponse()
{
	ssize_t	sent;
	std::ostringstream response_stream;
	std::string content;
	if (this->_errno == 1)
	{
		_response_string = this->getResponse().createError(404);
		_content_length = _response_string.size();
		sent = send(this->_response.getConnFd(), &_response_string[0], _response_string.size(), MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(404);
		this->_response.getRequest().setSentSize(sent);
	}
	else if (this->_errno == 2)
	{
		_response_string = this->getResponse().createError(503);
		_content_length = _response_string.size();
		sent = send(this->_response.getConnFd(), &_response_string[0], _response_string.size(), MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(503);
		this->_response.getRequest().setSentSize(sent);
	}
	else if (access(_response.getRequest().getUri().c_str(), R_OK) == -1 || access(_response.getRequest().getUri().c_str(), X_OK) == -1)
	{
		_response_string = this->getResponse().createError(403);
		_content_length = _response_string.size();
		sent = send(this->_response.getConnFd(), &_response_string[0], _response_string.size(), MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(403);
		this->_response.getRequest().setSentSize(sent);
	}
	// else if (exit_status.find(this->_pid)->second != 0 || this->_errno != 0)
	// {
	// 	_response_string = this->getResponse().createError(500);
	// 	_content_length = _response_string.size();
	// 	sent = send(this->_response.getConnFd(), &_response_string[0], _response_string.size(), MSG_DONTWAIT);
	// 	this->_response.getRequest().setStatusCode(500);
	// 	this->_response.getRequest().setSentSize(sent);
	// }
	else if (_content_length == 0)
	{
		_response_string = "HTTP/1.1 204 OK\r\nConnection: Keep-Alive\r\n\r\n";
		_content_length = _response_string.size();
		sent = send(this->_response.getConnFd(), &_response_string[0], _response_string.size(), MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(204);
		this->_response.getRequest().setSentSize(sent);
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
		sent = send(this->_response.getConnFd(), &_response_buff[this->_bytes_sent], _response_buff.size() - this->_bytes_sent, MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(200);
		this->_response.getRequest().setSentSize(sent);
	}
	else
	{
		sent = send(this->_response.getConnFd(), &_response_buff[this->_bytes_sent], _response_buff.size() - this->_bytes_sent, MSG_DONTWAIT);
		this->_response.getRequest().setStatusCode(200);
		this->_response.getRequest().setSentSize(sent);
	}
	if (sent >= 0)
	{
		this->_bytes_sent += sent;
		if (this->_bytes_sent == this->_content_length)
		{
			// exit_status.erase(this->_pid);
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
		std::cout << "request buff empty" << std::endl;
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
		if (this->_bytes_sent >= this->_content_length)
		{
			this->_body_complete = true;
			this->_bytes_sent = 0;
			this->_content_length = 0;
		}
	}
}

bool CGI::completeContent()
{
	if (getResponse().isChunked())
		return false;
	if (this->_request_buff.size() - this->_content_length == 0)
	{
		std::cout << "CGI CONTENT COMPLETE" << std::endl;
		return true;
	}
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

void CGI::mergeChunk(char *buffer, size_t received)
{
	size_t	pos = 0;
	while (pos < received)
	{
		if (this->_chunk_remaining == 0)
		{
			convertHex(buffer, pos, received);
			std::cout << buffer << std::endl;
			std::cout << "pos: " << pos << std::endl;
			std::cout << "received: " << received << std::endl;
			std::cout << "chunk remaining: " << this->_chunk_remaining << std::endl;
			std::cout << "buffer start" << std::endl;
			for (size_t i = 0; i < this->_request_buff.size(); i++)
				std::cout << this->_request_buff[i];
			std::cout << "buffer end" << std::endl;
			if (buffer[0] == 0)
				exit(0);
			if (this->_chunk_remaining == 0)
			{
				addHeaderChunked();
				this->_content_length += this->_header_length;
				this->_response.finishChunk();
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
			this->_chunk_remaining = 0;
		}
	}
}

// Convert hexadecimal number to decimal
void CGI::convertHex(char *buffer, size_t &pos, size_t received)
{
	char *stopstr;
	size_t i = pos;
	while (pos < received && buffer[pos] != '\r' && buffer[pos] != '\n')
		pos++;
	this->_chunk_remaining = std::strtoul(&buffer[i], &stopstr, 16);
	this->_content_length += this->_chunk_remaining;
	pos += 2;
}

void CGI::addHeaderChunked()
{
	std::map<std::string, std::string>::const_iterator it = this->_response.getRequest().getCompleteHeader().begin();
	std::ostringstream oss;
	oss << this->_content_length;
	std::string len(oss.str() + "\r\n\r\n");
	for (int i = len.length() - 1; i >= 0; i--)
	{
		this->_request_buff.insert(this->_request_buff.begin(), len[i]);
		this->_header_length++;
	}
	for (int i = 15; i >= 0; i--)
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
	this->_content_length = 0;
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

void CGI::setErrNo(int err)
{
	this->_errno = err;
}

// std::string	CGI::get_response_string()
// {
// 	return this->_response_string;
// }
