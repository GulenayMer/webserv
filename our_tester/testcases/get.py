import sys

sys.path.append("../")
from send_request import send_request
import config


# def test_get_dir_index() -> str:
#     request_header = "GET / HTTP/1.1\r\nHost: {}\r\n\r\n".format(config.SERVER_NAME)
#     http_response = send_request(request_header)
#     print("test_get_dir_index")
#     if http_response.status != 200:
#         return "Bad status code: {}, expected: {}".format(
#             str(http_response.status), "200"
#         )
#     body = http_response.read().decode("UTF-8")
#     if body != "google":
#         return "Bad content: {}, expected: {}".format(str(body), "google")
#     return ""