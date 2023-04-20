#!/bin/bash

while read line
do
	result=`echo $line | grep -c "file-to-delete"`
	if [ $result != 0 ]; then
		read line
		read line
		line="$(echo -e "$SERVER_NAME:$SERVER_PORT${line}" | sed -e 's/[[:space:]]*$//')"
		curl -s -X DELETE -i $line > /dev/null
		break
	fi
done < "${1:-/dev/stdin}"

message="<html>
<head>
<meta charset="UTF-8">
<title>Redirecting...</title>
<meta http-equiv='refresh' content='3;url=/index.html'>
</head>
<body>
<pre>
Deleted $line
</pre>
</body>
</html>
"
echo "HTTP/1.1 200 OK"
echo "Content-Length: ${#message}"
echo -e "Content-type: text/html\r\n\r"
echo -en "$message"
