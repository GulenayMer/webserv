#!/usr/bin/python3

import cgi, os, hashlib, json, cgitb
from dotenv import load_dotenv

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

def print_message(body, user_exists):
	body = "<body>" + body + "</body>"
	html = "<html>"
	html += "<head>"
	html += "<title>Redirecting...</title>"
	# Meaning user was created in this registration
	if user_exists == False:
		html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
		message = "HTTP/1.1 201 OK\r\n"
	else:
		html += "<meta http-equiv='refresh' content='0;url=/failedRegistration.html'>"
		message = "HTTP/1.1 409 OK\r\n"
	html += "</head>"
	html += body
	html += "</html>"
	message += f"Content-length: {len(html)} \r\n"
	message += "Content-type:text/html\r\n\r\n"
	message += html

	print(message)

load_dotenv()
try:
	cgitb.enable()
	form = cgi.FieldStorage()

	username = form["username"].file.read()
	password = form["password"].file.read()
	email = form["email"].file.read()

	user = {
		"username": username,
		"hash": str(hashlib.sha256(password.encode('utf-8')).hexdigest()),
		"email": email
	}
	body = ""
	user_exists = False
	db_path = os.environ["DB_PATH_WEBSITE"]
	# File does not exist, create it with a list and add first user
	if not os.path.exists(db_path[:-10]):
		os.mkdir(db_path[:-10])
	if not os.path.exists(db_path) or is_json_file_valid(db_path) == False:
		with open(db_path, 'w', encoding='utf-8') as db:
			json.dump([], db)
		with open(db_path, "r", encoding='utf-8') as db:
			users = json.load(db)
		with open(db_path, 'w', encoding='utf-8') as db:	
			users.append(user)
			json.dump(users, db)
			body = "User was created."
	# db file exists, check user
	else:
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
	print_message(body, user_exists)
except KeyError:
	print_message("There was a problem creating your account.", True)