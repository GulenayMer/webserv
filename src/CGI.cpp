# include "../include/CGI.hpp"

CGI::CGI(Config config, const std::string path, std::string response_body)
{
	this->_config = config;
	this->_path = path;
	this->_response_body = response_body;
}

CGI::CGI(const CGI& obj)
{
	*this = obj;
}

CGI& CGI::operator=(const CGI& obj)
{
	if (this != &obj) {
		this->_path = obj._path;
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

int		CGI::handle_cgi()//std::ostringstream &response_stream)
{
    std::ifstream file;
	int fd[2];
	std::string new_path = _path;
	std::string shebang;
	char buff[1000];
	memset(buff, 0, 1000); // TODO we need to initialize buff (otherwise gives cond.jump etc. error) (std::fill)
    if (pipe(fd) < 0)
    {
        std::cout << "Error opening pipe" << std::endl;
        return EXIT_FAILURE;
    }
	new_path = remove_end(_path, '?');
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
		waitpid(pid, NULL, 0);
		while (read(fd[0], buff, sizeof(buff) - 1)) {
			_response_body += buff;
			memset(buff, 0, 1000);
		}
		close(fd[0]);
		std::cout << _response_body << std::endl;
    }
	return EXIT_SUCCESS;
}

void	CGI::exec_script(int *pipe, std::string path, std::string program)
{
    char *args[2];
	size_t i = 0;
	size_t j = 0;
	close(pipe[0]);
    args[0] = new char [_config.get_cgi().get_path().find(program.c_str())->second.length() + 1];
    for (i = 0; i < _config.get_cgi().get_path().find(program.c_str())->second.length(); i++) {
        args[0][i] = _config.get_cgi().get_path().find(program.c_str())->second[i];
	}
	args[0][i] = '\0';
    args[1] = new char [path.length() + 1];
    for (j = 0; j < path.length(); j++) {
        args[1][j] = path[j];
	}
	args[1][j] = '\0';
    args[2] = NULL;
	dup2(pipe[1], STDOUT_FILENO);
	close(pipe[1]);
    execve(args[0], args, NULL);
    perror("execve failed.");
	exit(0);
}