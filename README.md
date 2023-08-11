## Project Name: Webserv
This project is about writing our own HTTP server in C++. The server is able to handle requests from web browsers and serve static websites.

## Summary
The goal of this project is to create a functional HTTP server that can handle various HTTP methods such as GET, POST, and DELETE. The server is non-blocking and uses a single poll() function for all I/O operations. It serves static websites, handles file uploads, and provides accurate HTTP response status codes. The project handles additional bonus features such as support for cookies and session management, and handling multiple CGI (Common Gateway Interface) programs.

## Usage
To compile the program, the provided Makefile can be used. There we defined some rules and they can be run by these basic commands:<br>
**make**: This command compiles the necessary source code files and links them to create the final executable.<br>
**make test**: This command runs valgrind for memory leaks and is used for debugging purposes.<br>
**make clean**: This command will remove any generated files and clean up the project directory.<br>

## Contributors 
jpa-rocha<br>
De1iad<br>
Juzanok<br>
libacchu<br>
GulenayMer<br>

