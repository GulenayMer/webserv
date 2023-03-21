#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Utils.hpp"
# include "MIME.hpp"
# include "CGI.hpp"
# include <cstring>
# include "Config.hpp"
# include "httpHeader.hpp"

class httpHeader;

class Response 
{

    private:
		std::string                 _httpVersion;
		std::string                 _response_number;
        int                         _conn_fd;
        int                         _server_fd;
		size_t						_bytes_sent;
		std::string					_req_uri;
		bool		                _is_cgi;
        MIME                        _types;
		std::string			        _response_body;
		std::string			        _respond_path;
		std::string			        _response;
		Config      	   			_config;
		httpHeader	 				_request;
    
        Response();

    public:
		Response(int conn_fd, int server_fd, Config& config);
        Response(Response const &cpy);
        Response &operator=(Response const &rhs);
        ~Response();

        int 	send_response();

        int		handle_cgi(const std::string& path, std::string& response_body, std::ostringstream &response_stream);
        void	exec_script(int *pipe, std::string path, std::string program);

        void 	send_404(std::string root, std::ostringstream &response_stream);

		void	new_request(httpHeader &request);

		void	responseToGET(std::ifstream &file, const std::string& path, std::ostringstream &response_stream);
		void	responseToPOST(const httpHeader request, std::ostringstream &response_stream);
		void	responseToDELETE(std::ostringstream &response_stream);
		bool	response_complete() const;
};

#endif