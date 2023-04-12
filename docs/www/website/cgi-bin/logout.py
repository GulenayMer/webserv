#!/usr/bin/python3

import cgi, os, hashlib, json, http.cookies
from dotenv import load_dotenv

cookie = http.cookies.SimpleCookie()
# set a value for the cookie
cookie['session'] = "logout"
cookie["session"]["path"] = "/"
cookie["session"]["max-age"] = 0
body = "User logout"
body = "<body>" + body + "</body>"

html = "<html>"
html += "<head>"
html += "<title>Redirecting...</title>"
html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
html += "</head>"
html += body
html += "</html>"

message = "HTTP/1.1 200 OK\r\n"
message += cookie.output()
message += "\n" # print a blank line to separate the headers from the body 
message += f"Content-length: {len(html)} \r\n"
message += "Content-type:text/html\r\n\r\n"

message += html

print(message)