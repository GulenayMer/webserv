#!/usr/bin/python3

import sys

try:
	f = open("uploaded.png", "xb")
except:
	print("HTTP/1.1 200 OK\r")
	print("Content-Type: text/html\r")
	print("Content-Length: 42\r")
	print("\r")
	print("<html><h1>File already exists.</h1></html>")
	sys.exit()

with f:
	for line in sys.stdin:
		if line == "\r\n":
			break
	for line in sys.stdin:
		f.write(line.encode('utf8','surrogateescape'))
f.close()

print("HTTP/1.1 204 No Content\r\n")
	