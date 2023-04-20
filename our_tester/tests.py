import requests
import config
import sys

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

def test_get(uri = None, expected_status = None) -> str:
    if uri is None:
        req = requests.get(get_base_url())
    else:
        req = requests.get(get_base_url() + uri)
    if req.status_code != expected_status:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), expected_status
        )
    return ""

def test_errors(uri = None, expected_status = None) -> str:
    if uri is None:
        req = requests.get(get_base_url())
    else:
        req = requests.get(get_base_url() + uri)
    if req.status_code != expected_status:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), expected_status
        )
    return ""

def test_post(uri = None, expected_status = None) -> str:
    if uri is None:
        req = requests.post(get_base_url())
    else:
        req = requests.post(get_base_url() + uri)
    if req.status_code != expected_status:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), expected_status
        )
    return ""

def test_delete(uri = None, expected_status = None) -> str:
    if uri is None:
        req = requests.delete(get_base_url())
    else:
        req = requests.delete(get_base_url() + uri)
    if req.status_code != expected_status:
        return "Bad status code: {}, expected: {}".format(
            str(req.status_code), expected_status
        )
    return ""