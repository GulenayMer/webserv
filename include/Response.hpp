#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Utils.hpp"
# include "MIME.hpp"
//# include "CGI.hpp"
# include <cstring>
# include "Config.hpp"
# include "httpHeader.hpp"

class httpHeader;

class Response 
{

    private:
		std::string                 _httpVersion;
		std::string                 _response_number;
		std::string					_addr;
        int                         _conn_fd;
        int                         _server_fd;
		size_t						_bytes_sent;
		struct pollfd*				_fds;
		int							_nfds;
		std::string					_req_uri;
		bool		                _is_cgi;
		bool						_is_complete;
		int							_cgi_fd;
        MIME                        _types;
		std::string			        _response_body;
		std::string			        _respond_path;
		std::string			        _response;
		std::string					_buffer;
		Config      	   			_config;
		httpHeader	 				_request;
		bool						_error;
		std::string					_location;
    
        Response();
		static std::string getErrorPath(int &errorNumber, std::string& errorName);

    public:
		Response(int conn_fd, int server_fd, Config& config, struct pollfd* fds, int nfds, std::string addr);
        Response(Response const &cpy);
        Response &operator=(Response const &rhs);
        ~Response();

        int 	send_response();
        void 	send_404(std::string root, std::ostringstream &response_stream);

		void	new_request(httpHeader &request);

		void	responseToGET(std::ifstream &file, const std::string& path, std::ostringstream &response_stream);
		void	responseToPOST(const httpHeader request, std::ostringstream &response_stream);
		void	responseToDELETE(std::ostringstream &response_stream);
		bool	response_complete() const;

		bool	is_cgi();
		int		getConnFd();
		Config &getConfig();
		httpHeader &getRequest();
		MIME	&getTypes();
		void	setCGIFd(int fd);
		int		getCGIFd();

		static std::string	createError(int errorNumber);
		void getPath();
		bool checkCGI();
		bool checkPermissions();
		void completeProg(bool complete);
		bool isComplete();
		std::string &getAddress();
};



#endif