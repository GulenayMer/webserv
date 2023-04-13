#ifndef MIME_HPP
# define MIME_HPP

# include <map>
# include <string>
	// IF MIME type is not allowed it segfaults
	class MIME {
		private:
			std::map<std::string, std::string> _mime_types;
		public:
			MIME();
			std::string get_content_type(std::string type);
	};

#endif