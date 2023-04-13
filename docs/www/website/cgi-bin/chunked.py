#!/usr/bin/python3

import sys

try:
	f = open("uploaded.txt", "x")
except:
	print("File already exists.")
	sys.exit()

with f:
	for line in sys.stdin:
		f.write(line)
f.close()
	