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

def print_message(body, user_exists, is_error):
	message = ""
	html = ""
	if is_error == True:
		body = "No user provided for login"
		user_exists = False
	body = "<body>" + body + "</body>"
	html = "<html>"
	html += "<head>"
	html += "<title>Redirecting...</title>"
	if user_exists == True:
		html += "<meta http-equiv='refresh' content='0;url=/index.html'>"
	else:
		html += "<meta http-equiv='refresh' content='0;url=/failedLogin.html'>"
	html += "</head>"
	html += body
	html += "</html>"
	message = "HTTP/1.1 200 OK\r\n"
	if user_exists == True:
		message += cookie.output()
		message += "\n" # print a blank line to separate the headers from the body 
	message += f"Content-length:{len(html)}\r\n"
	message += "Content-type:text/html\r\n\r\n"
	message += html
	print(message)

body = ""
user_exists = False
try:
	form = cgi.FieldStorage()
	username = form["username"].file.read()
	password = form["password"].file.read()
	user = {
		"username": username,
		"hash": str(hashlib.sha256(password.encode('utf-8')).hexdigest()),
	}
	# check if db file exists, if not create it
	db_path = os.environ["DB_PATH_WEBSITE"]
	if os.path.exists(db_path):
		if is_json_file_valid(db_path) == False:
			body = "User does not exist."
			user_exists = False
	# File does not exist, create it with a list and add first user
		else:
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
				cookie['session'] = user["username"]
				cookie["session"]["path"] = "/"
				cookie["session"]["max-age"] = 90000
			# if user does not exist create user
			else:
				body = "There was a problem accessing this account"
		print_message(body, user_exists, False)
except KeyError:
	print_message(body, user_exists, True)

