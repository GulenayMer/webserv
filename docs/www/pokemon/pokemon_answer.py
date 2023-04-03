#!/usr/bin/python3

import cgi

form = cgi.FieldStorage()
image = form["image"].file.read()
guess = form["guess"].file.read()
guess = guess.lower()

image_path = f"images/{image}-2.png"

if image == "diglett":
	image = "uncle ben"

body = "<!DOCTYPE html>\n<head>\n<title>Who's that Pokemon?</title>\n</head>\n<body>\n"
if image == guess:
	body += f"<h1>Congratulations, the answer was {image.title()}!</h1>\n"
else:
	body += f"<h1>Too bad! The answer was {image.title()}.</h1>\n"
body += f"<img src={image_path} alt=\"{image_path}\">"
body += "<form METHOD=GET ACTION=\"/pokemon/pokemon_question.py\">"
body += "<button type=\"submit\">Try again?</button>"
body += "</form>"
body += "<form METHOD=GET ACTION=\"/\">"
body += "<button type=\"submit\">Homepage</button>"
body += "</form>\n</body>\n</html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
