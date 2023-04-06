#!/usr/bin/python3

import cgi, os, hashlib, json, http.cookies
from dotenv import load_dotenv

load_dotenv()
form = cgi.FieldStorage()

# print(form)
username = form["username"].file.read()
password = form["password"].file.read()
body = ""
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
			user_exists = True
			break
	if user_exists == True:
		cookie = http.cookies.SimpleCookie()
		# set a value for the cookie
		cookie['username'] = user["username"]
		# TODO get domain from website?
		cookie["username"]["domain"] = "example.com"
		cookie["username"]["path"] = f"/{username}"
		cookie["username"]["max-age"] = 3600
	# if user does not exist create user
	else:
		body = "There was a problem accessing this account"
# File does not exist, create it with a list and add first user
else:
	body = "User does not exist."	

body = "<body>" + body + "</body>"

html = "<html>"
html += "<head>"
html += "<title>Redirecting...</title>"
html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
html += "</head>"
html += body
if user_exists == True:
	html += cookie.output()
html += "\n" # print a blank line to separate the headers from the body 
html += "</html>"

message = "HTTP/1.1 200 OK\r\n"
message += f"Content-length: {len(html)} \r\n"
message += "Content-type:text/html\r\n\r\n"
message += html

print(message)
