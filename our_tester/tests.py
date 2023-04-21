import requests
import config
import sys
import subprocess
import urllib.parse
import uuid
import os
from send_request import send_request

def get_base_url() -> str:
	"""
	Return the base URL of the server
	"""
	return "http://{}:{}/".format(config.SERVER_NAME, config.SERVER_PORT)

# def test_get() -> str:
#     req = requests.get(get_base_url())
#     if req.status_code != 200:
#         return "Bad status code."
#     return ""

def test_get(uri = None, expected_status = None, data_to_send = None) -> str:
	if uri is None:
		req = requests.get(get_base_url())
	else:
		req = requests.get(get_base_url() + uri)
	if req.status_code != expected_status:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), expected_status
		)
	return ""

def test_multiple_get() -> str:
	for i in range(100):
		req = requests.get(get_base_url())
		if req.status_code != 200:
			return "Bad request at {}th iteration.".format(i + 1)
	return ""

def test_errors(uri = None, expected_status = None, data_to_send = None) -> str:
	if uri is None:
		req = requests.get(get_base_url())
	else:
		req = requests.get(get_base_url() + uri)
	if req.status_code != expected_status:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), expected_status
		)
	return ""

def test_post(uri = None, expected_status = None, data_to_send = None) -> str:
	if uri is None:
		req = requests.post(get_base_url())
	else:
		if data_to_send != None:
			boundary = str(uuid.uuid4())
			headers = {"Content-type": f"multipart/form-data; boundary={boundary}"}
			data = []
			for key, value in data_to_send.items():
				if value is not None:
					data.append(
						f"--{boundary}\r\nContent-Disposition: form-data; name=\"{key}\"\r\n\r\n{value}\r\n"
					)
			data.append(f"--{boundary}--\r\n")
			req = requests.post(get_base_url() + uri, headers=headers, data="".join(data))
		else:
			req = requests.post(get_base_url() + uri)
	if req.status_code != expected_status:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), expected_status
		)
	return ""

# def test_cookies(uri = None, expected_status = None, data_to_send = None) -> str:
	# if uri is None:
	#     req = requests.post(get_base_url())
	# else:
	#     if data_to_send != None:
	#         cookies = data_to_send
	#         print(cookies)
			# req = requests.post(get_base_url() + uri, cookies=cookies)
	#     else:
	#         req = requests.post(get_base_url() + uri)
	# if req.status_code != expected_status:
	#     return "Bad status code: {}, expected: {}".format(
	#         str(req.status_code), expected_status
	#     )
	# return ""

def test_delete(uri = None, expected_status = None, data_to_send = None) -> str:
	if uri is None:
		req = requests.delete(get_base_url())
	else:
		if expected_status == 204:
			create_path = f"{data_to_send}"
			with open(create_path, 'w') as file:
				file.write('Hello, world, I will be delete soon')
		req = requests.delete(get_base_url() + uri)
	if req.status_code != expected_status:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), expected_status
		)
	return ""

def URITooLarge() -> str:
	req = requests.get(get_base_url() + 'a' * 2000)
	if req.status_code != 414:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), "414"
		)
	return ""

def HTTPVersionNotSupported() -> str:
	request_header = 'GET / HTTP/0.1\r\nHost: {}:{}\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
	http_response = send_request(request_header)
	if http_response.status != 505 and http_response.status // 100 != 4:
		return "Bad status code: {}, expected: {}".format(str(http_response.status), "505")
	return ""

def test_403() -> str:
	old_permisions = 0o644;
	new_permisions = 0o200;
	os.chmod("../docs/www/website/a/a.html", new_permisions);
	req = requests.get(get_base_url() + "a/a.html")
	os.chmod("../docs/www/website/a/a.html", old_permisions);
	if req.status_code != 403:
		return "Bad status code: {}, expected: {}".format(
			str(req.status_code), "403"
		)
	return ""

def test_500() -> str:
	request_header = 'GET /cgi-bin/invalid_for_testing.py HTTP/1.1\r\nHost: {}:{}\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
	http_response = send_request(request_header)
	try:
		if http_response.status != 500:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "500")
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("500")
	return ""

def chunk_gen(file, chunk_size):
	f = open(file, "rb")
	while True:
		data = f.read(chunk_size)
		if not data:
			break
		yield data

def test_chunked_text() -> str:
	headers = {"Transfer-Encoding":"chunked", "content-type":"text/plain"}
	http_response = requests.post(get_base_url() + "cgi-bin/chunked.py", headers=headers, data=chunk_gen("our_tester/resources/blah.txt", 5))
	try:
		if http_response.status_code != 200 and http_response.status_code != 204:
			return f"Bad status code: {str(http_response.status_code)}, expected 200 or 204"
		result = subprocess.run(["diff", "our_tester/resources/blah.txt", "docs/www/website/cgi-bin/uploaded.txt"])
		if result.returncode != 0:
			return "Uploaded file differs from original"
	except Exception as e:
		return f"Error: {e}"
	return ""

def test_chunked_img() -> str:
	headers = {"Transfer-Encoding":"chunked", "content-type":"image/png"}
	http_response = requests.post(get_base_url() + "cgi-bin/chunked.py", headers=headers, data=chunk_gen("our_tester/resources/chunked.png", 1024))
	try:
		if http_response.status_code != 200 and http_response.status_code != 204:
			return f"Bad status code: {str(http_response.status_code)}, expected 200 or 204"
		result = subprocess.run(["diff", "our_tester/resources/chunked.png", "docs/www/website/cgi-bin/uploaded.png"])
		if result.returncode != 0:
			return "Uploaded file differs from original"
	except Exception as e:
		return f"Error: {e}"
	return ""

def test_501() -> str:
	request_header = 'OPTIONS / HTTP/1.1\r\nHost: {}:{}\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
	http_response = send_request(request_header)
	if http_response.status != 501 and http_response.status // 100 != 4:
		return "Bad status code: {}, expected: {}".format(
			str(http_response.status), "501")
	return ""

def test_request_line_multiple_space() -> str:
	request_header = "GET  /  HTTP/1.1\r\nHost: {}:{}\r\n\r\n".format(config.SERVER_NAME, config.SERVER_PORT)
	http_response = send_request(request_header)
	try:
		if http_response.status != 400:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "400")
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("400")
	return ""

def test_post_not_allowed() -> str:
	request_header = "POST / HTTP/1.1\r\nHost: {}:{}\r\n\r\n".format(config.SERVER_NAME, config.SERVER_PORT)
	http_response = send_request(request_header)
	try:
		if http_response.status != 405:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "405")
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("405")
	return ""

def test_space_before_colon() -> str:
	request_header = "GET / HTTP/1.1\r\nHost :{}\r\n\r\n".format(config.SERVER_NAME)
	http_response = send_request(request_header)
	try:
		if http_response.status != 400:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "400")
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("400")

	request_header = (
		"GET / HTTP/1.1\r\nHost:{}\r\nAccept-Language :hyeyoo\r\n\r\n".format(
			config.SERVER_NAME
		)
	)
	http_response = send_request(request_header)
	try:
		if http_response.status != 400:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "400"
			)
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("400")
	return ""

def test_missing_header_name() -> str:
	request_header = "GET / HTTP/1.1\r\nHost:{}\r\n:empty_name\r\n\r\n".format(
		config.SERVER_NAME
	)
	http_response = send_request(request_header)
	try:
		if http_response.status // 100 != 4:
			return "Bad status code: {}, expected: {}".format(
				str(http_response.status), "4XX"
				)
	except Exception as e:
		print(f"Error: {e}")
		return "Bad status code, expected: {}".format("4XX")
	return ""