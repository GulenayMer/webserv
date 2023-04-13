#!/usr/bin/python3

import cgi, requests

form = cgi.FieldStorage()
pokemon = form["pokemon"].file.read()
pokemon = pokemon.lower()

response = requests.get(f"https://pokeapi.co/api/v2/pokemon/{pokemon}/")

body = "<!DOCTYPE html>\n<head>\n<link href=/pokemon/pstyle.css rel=stylesheet type=text/css>\n<title>Pokedex</title>\n</head>\n<body>\n"


if response.status_code == 200:
	data = response.json()
	sprite = data["sprites"]["front_default"]
	body += f"<img src={sprite} alt=/images/pokeball width=200 height=200>"
	body += f"<div>{pokemon.title()}</div>"
	body += f"<div>Abilities:<br/>"
	for ability in data["abilities"]:
		body += f"- {ability['ability']['name'].capitalize()}<br/>"
	body += "</div>"
else:
	body += f"<div>{pokemon.title()} not found.</div>"
body += f"<div id=d1>Search again:</div>"
body += "<div><form id=form action=/pokemon/pokedex.py method=post enctype=multipart/form-data>"
body += "<label for=pokemon>Enter a Pokemon:</label>"
body += "<input id=pokemon type=text name=pokemon required></input>"
body += "<button id=button class=button type=submit value=Submit>Submit</button>"
body += "</form></div>"
body += "<form METHOD=GET ACTION=/>"
body += "<button type=submit>Homepage</button>"
body += "</form></body></html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
