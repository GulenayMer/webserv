#!/usr/bin/python3

import cgi, requests

form = cgi.FieldStorage()
pokemon = form["pokemon"].file.read()
pokemon = pokemon.lower()

response = requests.get(f"https://pokeapi.co/api/v2/pokemon/{pokemon}/")

body = "<!DOCTYPE html>\n<head>\n<link href=/pokemon/pstyle.css rel=stylesheet type=text/css>\n"
evolution_info = ""

if response.status_code == 200:
	data = response.json()
	response = requests.get(f"https://pokeapi.co/api/v2/pokemon-species/{data['id']}/")
	if response.status_code == 200:
		evolution = response.json()
		body += f"<style>div {{background-color: {evolution['color']['name']}}}</style>\n"
		if evolution['evolves_from_species'] != None:
			evolution_info += "<div id=d3>Evolves from:<br/>"
			evolution_info += f"{evolution['evolves_from_species']['name'].capitalize()}<br/>"
			evolution_info += "</div>"
	body += "<title>Pokedex</title>\n</head>\n<body>\n"
	sprite = data["sprites"]["front_default"]
	body += f"<img src={sprite} alt=/images/pokeball width=200 height=200>"
	body += f"<div id=d3>{pokemon.title()}</div>"
	body += "<div class=flex-container id=d3>"
	body += "<div id=d3>Types:<br/>"
	for types in data["types"]:
		body += f"- {types['type']['name'].capitalize()}<br/>"
	body += "</div>"
	body += "<div id=d3>Abilities:<br/>"
	for ability in data["abilities"]:
		body += f"- {ability['ability']['name'].capitalize()}<br/>"
	body += "</div>"
	body += evolution_info
else:
	body += "<title>Pokedex</title>\n</head>\n<body>\n"
	body += f"<div id=d3>{pokemon.title()} not found.</div>"
body += "</div>"
body += f"<div id=d1>Search again:</div>"
body += "<div id=d2><form id=form action=/pokemon/pokedex.py method=post enctype=multipart/form-data>"
body += "<label for=pokemon>Enter a Pokemon:</label>"
body += "<input id=pokemon type=text name=pokemon required></input>"
body += "<button id=button class=button type=submit value=Submit>Submit</button>"
body += "</form></div>"
body += "<form METHOD=GET ACTION=/>"
body += "<button type=submit>Homepage</button>"
body += "</form></body></html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
