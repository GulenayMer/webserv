#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()

fileitem = form["data"]
if fileitem.filename:
   if os.path.exists(os.getcwd() + '/docs/www/website/storage/') == False:
      os.mkdir(os.getcwd() + '/docs/www/website/storage/')
   open(os.getcwd() + '/docs/www/website2/storage/' + os.path.basename(fileitem.filename), 'wb').write(fileitem.file.read())
   message = 'The file "' + os.path.basename(fileitem.filename) + '" was uploaded to ' + os.getcwd() + '/docs/www/website2/cgi-bin/tmp/'
else:
   message = 'Uploading Failed'
message = "<H1> " + message + " </H1>"
message += "<form METHOD=GET ACTION=\"/\">"
message += "<button type=\"submit\">Homepage</button>"
message += "</form>\n</body>\n</html>"
message = f"HTTP/1.1 200 OK\nContent-Length:{len(message)}\nContent-Type:text/html\r\n\r\n" + message
print(message)
