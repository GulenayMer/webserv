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

def test_delete(uri = None, expected_status = None, data_to_send = None) -> str:
    if uri is None:
        req = requests.delete(get_base_url())
    else:
        req = requests.delete(get_base_url() + uri)
    if req.status_code != expected_status:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), expected_status
        )
    return ""

def test_delete(uri = None, expected_status = None, data_to_send = None) -> str:
    if uri is None:
        req = requests.delete(get_base_url())
    else:
        print(get_base_url() + uri)
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
    print("Test 500 - Bad status code")
    request_header = 'GET /cgi-bin/invalid_for_testing.py HTTP/1.1\r\nHost: {}:{}\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
    # request_header = 'GET /pokemon/pokemon_question.py HTTP/1.1\r\nHost: {}:{}\r\nContent-Length: 1\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
    http_response = send_request(request_header)
    try:
        if http_response.status != 500:
            return "Bad status code: {}, expected: {}".format(
                str(http_response.status), "500")
    except Exception as e:
        print(f"Error: {e}")
        return "Bad status code, expected: {}".format("500")
    return ""

def test_501() -> str:
    request_header = 'OPTIONS / HTTP/1.1\r\nHost: {}:{}\r\n\r\n'.format(config.SERVER_NAME, config.SERVER_PORT)
    http_response = send_request(request_header)
    if http_response.status != 501 and http_response.status // 100 != 4:
        return "Bad status code: {}, expected: {}".format(
            str(http_response.status), "501")
    return ""
