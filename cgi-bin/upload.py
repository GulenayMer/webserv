#!/usr/bin/python3

import cgi, os
import sys

# for line in sys.stdin:
#     print(line)
form = cgi.FieldStorage()
sdawd
fileitem = form["data"]
if fileitem.filename:
   if os.path.exists(os.getcwd() + '/cgi-bin/tmp') == False:
      os.mkdir(os.getcwd() + '/cgi-bin/tmp/')
   open(os.getcwd() + '/cgi-bin/tmp/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/cgi-bin/tmp'
else:
   message = 'Uploading Failed'
print ("Content-Type:text/html\r\n")
print("<H1> " + message + " </H1>")
