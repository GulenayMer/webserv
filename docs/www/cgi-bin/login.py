#!/usr/bin/python3

import cgi, os, hashlib, json, http.cookies
from dotenv import load_dotenv

load_dotenv()
form = cgi.FieldStorage()

# print(form)
username = form["username"].file.read()
password = form["password"].file.read()

user = {
	"username": username,
	"hash": str(hashlib.sha256(password.encode('utf-8')).hexdigest()),
}
# check if db file exists, if not create it
db_path = os.environ["DB_PATH"]

if os.path.exists(db_path):
	user_exists = False
	with open(db_path, "r", encoding='utf-8') as db:
		users = json.load(db)
	# check if user exists or email is taken
	for entry in users:
		# User exists
		if entry["username"] == user["username"] and entry["hash"] == user["hash"]:
			print("There was a problem creating your account.")
			user_exists = True
			break
	if user_exists == True:
		cookie = http.cookies.SimpleCookie()
		# set a value for the cookie
		cookie['username'] = user["username"]
		cookie["username"]["domain"] = "example.com"
		cookie["username"]["path"] = "/"
		cookie["username"]["max-age"] = 3600
	# if user does not exist create user
	else:
		print("User does not exist.")	
# File does not exist, create it with a list and add first user
else:
	print("User does not exist.")	

print("Content-Type: text/html")
print(cookie.output())
print()  # print a blank line to separate the headers from the body
print("<html><body><h1>Cookie sent!</h1></body></html>")