#!/usr/bin/python3

import cgi, os, hashlib, json, cgitb
from dotenv import load_dotenv

load_dotenv()
cgitb.enable()
form = cgi.FieldStorage()

username= form["username"].file.read()
password = form["password"].file.read()
email = form["email"].file.read()
user = {
	"username": username,
	"hash": str(hashlib.sha256(password.encode('utf-8')).hexdigest()),
	"email": email
}
# check if db file exists, if not create it
db_path = os.environ["DB_PATH"]

if os.path.exists(db_path):
	user_exists = False
	with open(db_path, "r", encoding='utf-8') as db:
		users = json.load(db)
	# check if user exists or email is taken
	for entry in users:
		if entry["username"] == user["username"] or entry["email"] == user["email"]:
			body = "There was a problem creating your account."
			user_exists = True
			break
		# if user does not exist create user
	if user_exists == False:
		users.append(user)
		with open(db_path, 'w', encoding='utf-8') as db:
			json.dump(users,db)

# File does not exist, create it with a list and add first user
else:
	with open(db_path, 'w', encoding='utf-8') as db:
		json.dump([], db)
	with open(db_path, "r", encoding='utf-8') as db:
		users = json.load(db)
	with open(db_path, 'w', encoding='utf-8') as db:	
		users.append(user)
		json.dump(users, db)
		body = "User was created."

body = "<body>" + body + "</body>"

html = "<html>"
html += "<head>"
html += "<title>Redirecting...</title>"
html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
html += "</head>"
html += body
html += "</html>"

message = "HTTP/1.1 200 OK\r\n"
message += f"Content-length: {len(html)} \r\n"
message += "Content-type:text/html\r\n\r\n"
message += html

print(message)