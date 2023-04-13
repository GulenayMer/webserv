#!/usr/bin/python3

import os
import random

choices = ["bellsprout", "charmander", "vulpix", "lugia", "piloswine", "rayquaza", "diglett", "moltres"]
choice = random.choice(choices)

image = "images/" + choice + ".png"

body = "<!DOCTYPE html>\n<head>\n<link href=/pokemon/pstyle.css rel=stylesheet type=text/css>\n<title>Who's that Pokemon?</title>\n</head>\n<body>\n"
body += "<h1>Who's that Pokemon?</h1>\n"
body += f"<img src={image} alt=\"{image}\">\n"
body += "<form id=form action=/pokemon/pokemon_answer.py method=post enctype=multipart/form-data>\n"
body += "<label id=l1 for=guess>Take a guess:</label>\n"
body += "<input id=guess type=text name=guess></input>\n"
body += f"<input type=hidden value={choice} name=image>\n"
body += "<button id=button class=button type=submit value=Submit>Submit</button>"
body += "</form>\n</body>\n</html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n{body}"
print(message)
