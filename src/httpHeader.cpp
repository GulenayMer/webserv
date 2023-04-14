#include "../include/httpHeader.hpp"

httpHeader::httpHeader()
{
	this->_content_length = 0;
}

httpHeader::httpHeader(std::string header)
{
	size_t start = 0, end = 0;
	this->_header_length = header.find("\r\n\r\n") + 4;
	end = header.find(" ");
	std::string tmp_method = header.substr(start, end - start);
	start = end + 1;
	end = header.find(" ", start);
	if (end - start > 2048)
	{
		this->_error = 1;
		this->_uri.clear();
	}
	else
	{
		this->_error = 0;
		this->_uri = decodeURI(header.substr(start, end - start));
		start = this->_uri.find_first_of("?");
		if (start != std::string::npos)
		{
			this->_query = this->_uri.substr(start);
			this->_uri.erase(start);
		}
	}
	start = end + 1;
	end = header.find("\r\n", start);
	this->_version = header.substr(start, end - start);
	setMethod(tmp_method);
	start = end + 1;
	while ((end = header.find("\r\n", start)) != std::string::npos)
	{
		std::string line = header.substr(start, end - start);
		size_t separator = line.find(": ");
		if (separator != std::string::npos) {
			std::string name;
			if (line[0] == '\n')
				name = line.substr(1, separator - 1);
			else
				name = line.substr(0, separator);
			std::string value = line.substr(separator + 2);
			name = toLowerCase(name);
			std::cout << RED << "name: " << name << " value: " << value << RESET << std::endl;
			this->setHeader(name, value);
		}
		else
			break;
		start = end + 2;
	}
	this->_content_length = atol(this->get_single_header("content-length").c_str());
}

httpHeader::~httpHeader()
{}

httpHeader::httpHeader(const httpHeader& copy)
{
	*this = copy;
}

httpHeader &httpHeader::operator=(const httpHeader& rhs)
{
	if (this != &rhs)
	{
		this->_header = rhs._header;
		this->_method = rhs._method;
		this->_uri = rhs._uri;
		this->_version = rhs._version;
		this->_header_length = rhs._header_length;
		this->_content_length = rhs._content_length;
		this->_cookie = rhs._cookie;
		this->_error = rhs._error;
	}
	return *this;
}

const httpMethods &httpHeader::getMethod() const
{
	return(this->_method);
}

const std::string &httpHeader::getUri() const
{
	return(this->_uri);
}

const std::string &httpHeader::getVersion() const
{
	return(this->_version);
}

const uint8_t &httpHeader::isError() const
{
	return this->_error;
}

const std::string httpHeader::get_single_header(std::string entry)
{
	std::string empty = "";
	std::map<std::string, std::string>::iterator it = this->_header.find(entry);
	if (it != this->_header.end()) {
		return it->second;
	}
	return empty;
}

void httpHeader::setHeader(std::string name, std::string value)
{
	this->_header.insert(std::map<std::string, std::string>::value_type(name, value));
	//_header[ name ] = value;
}

void httpHeader::setMethod(std::string tmp_method)
{
	int i = 0;
	std::string methods[9] = {"GET", "POST", "DELETE", "PUT", \
		"HEAD", "OPTIONS", "TRACE", "CONNECT", "NONE"};
	
	for (i = 0; i < 9; i++)
	{
		if (tmp_method == methods[i])
			break;
	}
	switch (i)
	{
		case 0:
			this->_method = GET;
			break;
		case 1:
			this->_method = POST;
			break;
		case 2:
			this->_method = DELETE;
			break;
		case 3:
			this->_method = PUT;
			break;
		case 4:
			this->_method = HEAD;
			break;
		case 5:
			this->_method = OPTIONS;
			break;
		case 6:
			this->_method = TRACE;
			break;
		case 7:
			this->_method = CONNECT;
			break;
		default:
			this->_method = NONE;
			// std::cerr << "Error method not compatible!" << std::endl; 
			break;
	}
	//_method = method;
}

void httpHeader::setVersion(std::string version)
{
	//TODO - check is version is valid
	// check for http 1.1
	// throw error
	_version = version;
}

void httpHeader::printHeader()
{
	std::string method;
	switch (_method) {
		case 0:
			method = "GET";
			break;
		case 1:
			method = "POST";
			break;
		case 2:
			method = "DELETE";
			break;
		case 3:
			method = "PUT";
			break;
		case 4:
			method = "HEAD";
			break;
		case 5:
			method = "OPTIONS";
			break;
		case 6:
			method = "TRACE";
			break;
		case 7:
			method = "CONNECT";
			break;
		case 8:
			method = "NONE";
			break;				
	}
	std::cout << GREEN <<"Method: " << YELLOW << method << RESET << std::endl;
    std::cout << GREEN << "URI: " << YELLOW << _uri << RESET << std::endl;
    std::cout << GREEN << "HTTP Version: " << YELLOW << _version << RESET << std::endl;

	std::cout << "map Contains:\n";
	for ( std::map<std::string, std::string>::iterator itr = _header.begin(); itr != _header.end(); ++itr)
	{
		std::cout << GREEN << itr->first << RESET << ": " << YELLOW << itr->second.c_str() << RESET << std::endl;
	}
}

/* check if header is http 1.1 protocol */
bool httpHeader::isHttp11()
{
	std::cout << "VERSION: " << this->_version << std::endl;
	if ((this->_version == "HTTP/1.1" || this->_version == "http/1.1" || this->_version == "Http/1.1") \
		&& !get_single_header("host").empty())
		return true;
	std::cout << "HERE INVALID HTTP" << std::endl;
	return false;
}

size_t httpHeader::getHeaderLength()
{
	return this->_header_length;
}

size_t httpHeader::getContentLength()
{
	return this->_content_length;
}

void	httpHeader::setURI(std::string str)
{
	this->_uri = str;
}

const std::map<std::string, std::string>& httpHeader::getCompleteHeader() const
{
	return this->_header;
}

int httpHeader::getPort()
{
	std::map<std::string, std::string>::iterator it = this->_header.find("host");
	if (it != this->_header.end())
	{
		size_t pos = it->second.find_first_of(":");
		if (pos != std::string::npos)
		{
			pos++;
			char *end = NULL;
			return strtoul(it->second.substr(pos).c_str(), &end, 10);
		}
	}
	return -1;
}
