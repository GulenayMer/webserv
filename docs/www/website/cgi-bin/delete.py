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


error = False
# File does not exist, or file is empty
# then user to delete does not exist, error is set to True
db_path = os.environ["DB_PATH_WEBSITE"]
if not os.path.exists(db_path[:-10]):
	error = True
if not os.path.exists(db_path) or is_json_file_valid(db_path) == False:
	error = True

# Check if the cookie exists, if yes extract username,
# else set error to True
cookie = http.cookies.SimpleCookie(os.environ.get("HTTP_COOKIE"))
if "session" in cookie:
    # Extract the username from the session value
    username = cookie["session"].value
else:
	cookie = http.cookies.SimpleCookie()
	# set a value for the cookie
	cookie['session'] = "logout"
	cookie["session"]["path"] = "/"
	cookie["session"]["max-age"] = 0
	error = True

# If error is still false proceed with user and cookie deletion
if error == False:
	# set a value for the cookie deletion
	cookie["session"]["path"] = "/"
	cookie["session"]["max-age"] = 0
	user = {
		"username": username,
	}

	# find user in users_db
	user_exists = False
	with open(db_path, "r", encoding='utf-8') as db:
		users = json.load(db)
	# check if user exists, if yes remove them
	for entry in users:
		if entry["username"] == user["username"]:
			body = "Found user."
			users.remove(entry)
			user_exists = True
			break

	# reset db without deleted user
	if user_exists == True:
		with open(db_path, 'w', encoding='utf-8') as db:
			json.dump(users,db)

	#  Prepare http response
	body = "User deleted."
	body = "<body>" + body + "</body>"

	html = "<html>"
	html += "<head>"
	html += "<title>Redirecting...</title>"
	html += "<meta http-equiv='refresh' content='0;url=/userDeleted.html'>"
	html += "</head>"
	html += body
	html += "</html>"

	message = "HTTP/1.1 200 OK\r\n"
	message += cookie.output()
	message += "\n" # print a blank line to separate the headers from the body 
	message += f"Content-length: {len(html)} \r\n"
	message += "Content-type:text/html\r\n\r\n"

	message += html

#  Prepare http response in case error is TRUE
#  TODO where should it redirect to?
else:
	body = "User does not exist."
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