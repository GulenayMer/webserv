#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()
fileitem = form["filename"]
print(fileitem.filename)