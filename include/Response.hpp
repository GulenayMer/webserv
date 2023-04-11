#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include "Utils.hpp"
# include "MIME.hpp"
# include <cstring>
# include "Config.hpp"
# include "httpHeader.hpp"
# include <dirent.h>
# include <sys/stat.h>

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
		bool		                _is_cgi;
		bool						_is_chunked;
		bool						_is_dir;
		bool						_list_dir;
		bool						_is_complete;
		bool						_to_close;
		int							_cgi_fd;
        MIME                        _types;
		std::string			        _response_body;
		std::string			        _respond_path;
		std::string			        _response;
		Config      	   			_config;
		httpHeader	 				_request;
		std::string					_ext;
		bool						_error;
		Location					_location;
		size_t						_received_bytes;
    
        Response();
		static std::string getErrorPath(int &errorNumber, std::string& errorName, Config* config);

    public:
		Response(int conn_fd, int server_fd, Config& config, struct pollfd* fds, int nfds, std::string addr);
        Response(Response const &cpy);
        Response &operator=(Response const &rhs);
        ~Response();

        int 	send_response();
        void 	send_404(std::string root, std::ostringstream &response_stream);

		bool	new_request(httpHeader &request);

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
		std::string &getExt();

		static std::string	createError(int errorNumber, Config* config);
		void getPath();
		bool directoryExists(const char* path);
		std::string directoryListing(std::string uri);
		bool checkCGI();
		bool checkPermissions();
		void setCompletion(bool complete);
		bool isComplete();
		std::string &getAddress();
		bool shouldClose();
		bool dir_exists(const std::string& dirName_in);
		ssize_t receivedBytes(ssize_t received);
		void setChunked();
		bool isChunked();
		void finishChunk();
};

#endif