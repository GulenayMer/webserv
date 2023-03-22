#ifndef HTTPHEADER_HPP
# define HTTPHEADER_HPP

# include "Utils.hpp"

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
		std::string _version;
		std::map<std::string, std::string> _header;
		std::string	_boundary;
	public:
		httpHeader();
		httpHeader(std::string header);
		~httpHeader();
		httpHeader(const httpHeader& copy);
		httpHeader& operator=(const httpHeader& rhs);

		const httpMethods& getMethod() const;
		const std::string& getUri() const;
		const std::string& getVersion() const;
		//const std::map<std::string, std::string>& get_complete_header() const;
		const std::string get_single_header(std::string entry);

		void setHeader(std::string name, std::string value);
		void setMethod(std::string tmp_method);
		void setVersion(std::string version);

		void printHeader();
};

#endif

/* 
GET /home.html HTTP/1.1 \r\n
Host: developer.mozilla.org
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10.9; rv:50.0) Gecko/20100101 Firefox/50.0
Accept: text/html,application/xhtml+xml,application/xml;q=0.9
Accept-Language: en-US,en;q=0.5
Accept-Encoding: gzip, deflate, br
Referer: https://developer.mozilla.org/testpage.html
Connection: keep-alive
Upgrade-Insecure-Requests: 1
If-Modified-Since: Mon, 18 Jul 2016 02:36:04 GMT
If-None-Match: "c561c68d0ba92bbeb8b0fff2a9199f722e3a621a"
Cache-Control: max-age=0 */
