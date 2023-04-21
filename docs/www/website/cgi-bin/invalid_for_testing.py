#!/usr/bin/python3

import os
body = "Invalid"
body = "<body>" + body + "</body>"

html = "<html>"
html += "<head>"
html += "<title>Invalid...</title>"
html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
html += "</head>"
html += body
html += "</html>"

message = "HTTP/1.1 200 OK\r\n"
message += f"Content-length: {len(html)} \r\n"
message += "Content-type:text/html\r\n\r\n"
message += html
        # invalid syntax for python 3.5
        print(message)