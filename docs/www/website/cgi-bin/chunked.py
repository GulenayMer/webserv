#!/usr/bin/python3

import sys
from mimetypes import guess_extension

ftype = ""
for line in sys.stdin:
	if line.find("content-type:") != -1:
		ftype = line[line.find(" ") + 1:len(line) - 1]
	if line == "\r\n":
		break
buffer = ""
for line in sys.stdin:
	buffer = buffer + line
ext = guess_extension(ftype, strict=False)
try:
	if ext is not None:
		f = open(f"uploaded{ext}", "xb")
	else:
		f = open(f"uploaded.txt", "xb")
except:
	print("HTTP/1.1 200 OK\r")
	print("Content-Type: text/html\r")
	print("Content-Length: 42\r")
	print("\r")
	print("<html><h1>File already exists.</h1></html>")
	sys.exit()

with f:
	for line in buffer:
		f.write(line.encode('utf8','surrogateescape'))
f.close()

print("HTTP/1.1 204 No Content\r\n\r")
	