#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()

username = form["username"].file.read()
email = form["email"].file.read()
password = form["password"].file.read()

# check if db file exists, if not create it
db_path = os.getcwd() + '/docs/www/user_db/users.csv
if os.oath.exists(db_path):
	# check if user exists

	# if user does not exist create user
else:
	with open(file_path, "w") as db:
        f.write("index,username,email,hash")
        print("User Database was created.")

body = "<html><head><link rel=stylesheet href=/utils/style.css><title>Data Submitted</title></head><body>"
body += "<h1>Form Submitted.</h1>"
body += f"<h2>The email for the user {username}, \"{email}\" has been taken and will be sold to spammers.</h2>"
body += "<form METHOD=GET ACTION=\"/\">"
body += "<button type=\"submit\">Homepage</button>"
body += "</form>\n</body>\n</html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
