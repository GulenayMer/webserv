#!/usr/bin/python3

import requests, cgi, os
from dotenv import load_dotenv

load_dotenv()

try:
	form = cgi.FieldStorage()

	message = form["message"].file.read()

except KeyError:
	message = "Hello"
	
url = "https://openai80.p.rapidapi.com/chat/completions"

payload = {
	"model": "gpt-3.5-turbo",
	"messages": [
		{
			"role": "user",
			"content": f"{message}"
		}
	]
}
headers = {
	"content-type": "application/json",
	"X-RapidAPI-Key": os.environ['API_KEY'],
	"X-RapidAPI-Host": "openai80.p.rapidapi.com"
}

response = requests.request("POST", url, json=payload, headers=headers)

body = "<!DOCTYPE html>\n<html>\n<head>\n<meta charset=UTF-8>\n<link href=/utils/cstyle.css rel=stylesheet type=text/css>\n<title>ChatBot</title>\n</head>\n<body>\n"
body += "<h1>Ask ChatGPT a question</h1>"
body += f"<div id=d1>{message}</div>"
body += "<div id=d2>"
if response.status_code == 200:
	data = response.json()
	body += data["choices"][0]["message"]["content"]
else:
	body += "ChatGPT decided to ignore you."
body += "</div>"
body += "<form id=form action=/cgi-bin/chat.py method=post enctype=multipart/form-data>"
body += "<label for=message>Enter a message</label>"
body += "<textarea id=message type=text name=message required></textarea><br/>"
body += "<button id=button class=button type=submit value=Submit>Submit</button>"
body += "</form>"
body += "<form METHOD=GET ACTION=\"/\">"
body += "<button type=\"submit\">Homepage</button>"
body += "</form>\n</body>\n</html>"

message = f"HTTP/1.1 200 OK\nContent-Length:{len(body)}\nContent-Type:text/html\r\n\r\n" + body
print(message)
