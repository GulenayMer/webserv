#ifndef HTTPHEADER_HPP
# define HTTPHEADER_HPP

# include "Utils.hpp"
# include "minilib.hpp"
# include <cstdlib>

// Set-Cookie: key=value; expires='formated date'; asdfaf=sadadd

enum httpMethods 
{
	GET,
	POST,
	DELETE,
	PUT,
	HEAD,
	OPTIONS,
	TRACE,
	CONNECT,
	NONE
};

class httpHeader
{
	private:
		httpMethods _method;
		std::string _uri;
		std::string _query;
		std::string _version;
		std::map<std::string, std::string> _header;
		size_t		_header_length;
		size_t		_content_length;
		std::string	_cookie;
		uint8_t		_error;
		
	public:
		httpHeader();
		httpHeader(std::string header);
		~httpHeader();
		httpHeader(const httpHeader& copy);
		httpHeader& operator=(const httpHeader& rhs);

		const httpMethods& getMethod() const;
		const std::string& getUri() const;
		const std::string& getVersion() const;
		const uint8_t &isError() const;
		const std::map<std::string, std::string>& getCompleteHeader() const;
		const std::string get_single_header(std::string entry);
		int getPort();
		void setHeader(std::string name, std::string value);
		void setMethod(std::string tmp_method);
		void setVersion(std::string version);
		void setURI(std::string str);
		size_t getHeaderLength();
		size_t getContentLength();
		bool isHttp11();

		void printHeader();
};

#endif
