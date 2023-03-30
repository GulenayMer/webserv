#!/usr/bin/python3

import cgi, os
import sys

# for line in sys.stdin:
#     print(line)
form = cgi.FieldStorage()

fileitem = form["data"]
if fileitem.filename:
   if os.path.exists(os.getcwd() + '/cgi-bin/tmp') == False:
      os.mkdir(os.getcwd() + '/cgi-bin/tmp/')
   open(os.getcwd() + '/cgi-bin/tmp/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
   message = 'Uploading Failed'
message = "<H1> " + message + " </H1>"
message = f"HTTP/1.1 200 OK\nContent-Length:{len(message)}\nContent-Type:text/html\r\n\r\n" + message
print(message)
