#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>

std::string decodeURI(const std::string &uri)
{
  std::ostringstream decoded;
  for (int i = 0; i < uri.length(); i++) {
    char c = uri[i];
    if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
      decoded << c;
      continue;
    }
    decoded << std::uppercase << '%' << std::setw(2) << int((unsigned char) c);
    decoded << std::nouppercase;
  }
  return decoded.str();
}