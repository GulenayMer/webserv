#include <iostream>
#include <fstream>
#include <dirent.h>
#include "../include/Response.hpp"
#include <sys/stat.h>

bool directoryExists(const char* path)
{
    struct stat info;
    if (stat(path, &info) != 0)
        return false;
    else if (info.st_mode & S_IFDIR)
        return true;
    else
        return false;
}

std::string directoryLisiting(std::string uri)
{
    DIR *dir;
    struct dirent *ent;

	if (!directoryExists(uri.c_str()))
		return (Response::createError(400));
	
	std::ostringstream outfile;

    outfile << "<!DOCTYPE html>\n";
    outfile << "<html>\n";
    outfile << "<head>\n";
    outfile << "<title>Directory Listing</title>\n";
    outfile << "</head>\n";
    outfile << "<body>\n";
    outfile << "<h1>Directory Listing</h1>\n";
    outfile << "<ul>\n";

    if ((dir = opendir(uri.c_str())) != NULL)
	{
        while ((ent = readdir(dir)) != NULL)
		{
            outfile << "<li><a href=\"" << uri << "/" << ent->d_name << "\" >" << ent->d_name << "</a></li>" << std::endl;
        }
        closedir(dir);
    }

    outfile << "</ul>\n";
    outfile << "</body>\n";
    outfile << "</html>\n";

	return (outfile.str());
}
