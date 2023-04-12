#!/usr/bin/python3

import cgi, os, hashlib, json, http.cookies
from dotenv import load_dotenv

load_dotenv()
def is_json_file_valid(file_path):
    with open(file_path, 'r') as f:
        contents = f.read()
        if len(contents) == 0:
            return False
        try:
            json.loads(contents)
        except ValueError:
            return False
        return True


cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE"))

# get user name from request
if "session" in cookie:
    # Extract the username from the session value
    username = cookie["session"].value
else:
	username = None
	error = True

user = {
	"username": username,
}
# find user in users_db
db_path = os.environ["DB_PATH"]
# File does not exist, create it with a list and add first user
error = False
if not os.path.exists(db_path[:-10]):
	error = True
if not os.path.exists(db_path) or is_json_file_valid(db_path) == False:
	error = True
# db file exists, check user
else:
	user_exists = False
	with open(db_path, "r", encoding='utf-8') as db:
		users = json.load(db)
	# check if user exists or email is taken
	for entry in users:
		if entry["username"] == user["username"]:
			body = "Found user."
			users.remove(entry)
			user_exists = True
			break
	# delete user
	if user_exists == True:
		print(users)
		
		with open(db_path, 'w', encoding='utf-8') as db:
			json.dump(users,db)


# set a value for the cookie
cookie["session"]["path"] = "/"
cookie["session"]["max-age"] = 0

#  Prepare http response
body = "User deleted."
body = "<body>" + body + "</body>"

html = "<html>"
html += "<head>"
html += "<title>Redirecting...</title>"
html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
html += "</head>"
html += body
html += "</html>"

message = "HTTP/1.1 200 OK\r\n"
message += cookie.output()
message += "\n" # print a blank line to separate the headers from the body 
message += f"Content-length: {len(html)} \r\n"
message += "Content-type:text/html\r\n\r\n"

message += html

print(message)