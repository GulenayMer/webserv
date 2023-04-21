import requests
import config
import sys
import subprocess

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
        data = ""
        if data_to_send != None:
            data = f"{data_to_send}"
        req = requests.post(get_base_url() + uri, data=data)
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

def URITooLarge() -> str:
    req = requests.get(get_base_url() + "a" * 1000)
    if req.status_code != 414:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), "414"
        )
    return ""

def HTTPVersionNotSupported() -> str:

    # url = get_base_url()
    # command = ['curl', '--http1.0', url]
    # result = subprocess.run(command, capture_output=True)
    # print(result.stdout)

    
    # req = requests.get(get_base_url(), headers={'HTTP_VERSION': 'HTTP/2'})
    # if req.status_code != 505:
    #     return "Bad status code: {}, expected: {}".format(
    #         str(req.status_code), "505"
    #     )
    request_header = 'GET / HTTP/0.1\r\nHost:{}\r\n\r\n'.format(config.SERVER_NAME)
    http_response = send_request(request_header)
    if http_response.status != 505 and http_response.status // 100 != 4:
        return
    return ""
