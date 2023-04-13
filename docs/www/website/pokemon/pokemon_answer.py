#!/usr/bin/python3

import cgi, requests

form = cgi.FieldStorage()
image = form["image"].file.read()
guess = form["guess"].file.read()
guess = guess.lower()


response = requests.get(f"https://pokeapi.co/api/v2/pokemon/{image}/")

if image == "diglett" and guess != "uncle ben":
	image = "uncle ben"
	image_path = f"images/diglett-3.png"
else:
	image_path = f"images/{image}-2.png"

body = "<!DOCTYPE html>\n<head>\n<link href=/pokemon/pstyle.css rel=stylesheet type=text/css>\n<title>Who's that Pokemon?</title>\n</head>\n<body>\n"
if image == guess:
	body += f"<h1>Congratulations, the answer was {image.title()}!</h1>\n"
else:
	if image == "uncle ben":
		body += f"<h1>That's right! The answer is {image.title()}.</h1>\n"
	else:
		body += f"<h1>Too bad! The answer was {image.title()}.</h1>\n"
body += f"<img src={image_path} alt=\"{image_path}\">"
if response.status_code == 200:
	data = response.json()
	body += f"<div>{image.title()}'s abilities include:<br/>"
	for ability in data["abilities"]:
		body += f"- {ability['ability']['name'].capitalize()}<br/>"
	body += "</div>"
body += "<form METHOD=GET ACTION=\"/pokemon/pokemon_question.py\">"
body += "<button type=\"submit\">Try again?</button>"
body += "</form>"
body += "<form METHOD=GET ACTION=\"/\">"
body += "<button type=\"submit\">Homepage</button>"
body += "</form>\n</body>\n</html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
