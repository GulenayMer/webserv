import os
import sys

from send_request import send_request

from typing import Callable
import config
from tests import *
from testcases.get import *

# import colors
# color letters
RESET = "\033[0m"

C_BLACK = "\033[30m"
C_RED = "\033[31m"
C_GREEN = "\033[32m"
C_YELLOW = "\033[33m"
C_BLUE = "\033[34m"
C_MAGENTA = "\033[35m"
C_CYAN = "\033[36m"
C_WHITE = "\033[37m"
C_GRAY = "\033[90m"
# color letters Gras
C_B_BLACK = "\033[30;01m"
C_B_RED = "\033[31;01m"
C_B_GREEN = "\033[32;01m"
C_B_YELLOW = "\033[33;01m"
C_B_BLUE = "\033[34;01m"
C_B_MAGENTA = "\033[35;01m"
C_B_GRAY = "\033[36;01m"
C_B_WHITE = "\033[37;01m"
C_B_GRAY = "\033[90;01m"
# color background
B_BLACK = "\033[40m"
B_RED = "\033[41m"
B_GREEN = "\033[42m"
B_YELLOW = "\033[43m"
B_BLUE = "\033[44m"
B_MAGENTA = "\033[45m"
B_GRAY = "\033[46m"
B_WHITE = "\033[47m"
B_GRAY = "\033[100m"

def cleanup() -> None:
    """
    Remove file created by the tester to make sure the test is new every run.
    """
    os.system("rm -rf www/tmp/*")
    os.system("rm -rf www/long.txt")

def run_test(test_name: str, test: Callable, uri = None, expected_status = None) -> None:
    """
    Runs a test defined in function test, with the name test_name,
    and prints weather it passed or not.
    """
    try:
            result = test(uri, expected_status)
    except:
        print(
            "{}Cannot connect to the server on port {}{}".format(
                C_B_RED, config.SERVER_PORT, RESET
            )
        )
        exit(1)
    char = ""
    color = C_GREEN
    if len(result) == 0:
        char = "✅"
    else:
        color = C_RED
        char = "❌"
    print(r"{:40} {}{} {}{}".format(test_name, color, char, result, RESET))

def run() -> None:
    """
    Entrypoint of the tester
    """
    
    print(r"{}{}### TESTING GET ###{}".format(C_B_WHITE, B_GRAY, RESET))
    run_test("Test 1: GET /", test_get, None, 200)
    run_test("Test 2: GET /pokemon", test_get, "pokemon", 200)
    run_test("Test 3: GET /pokemon/index.html", test_get, "pokemon/index.html", 200)
    run_test("Test 4: GET /contact", test_get, "contact", 200)
    run_test("Test 5: GET /about", test_get, "about", 200)
    run_test("Test 5: GET /fake", test_get, "fake", 200)


    print(r"{}{}### TESTING POST ###{}".format(C_B_WHITE, B_GRAY, RESET))
    run_test("Test 1: POST /pokemon", test_post, "pokemon", 200)

    # print(r"{}{}### TESTING DELETE ###{}".format(C_B_WHITE, B_GRAY, RESET))
    # run_test("Test 2: DELETE /pokemon", test_delete, "pokemon", 200)


    print(r"{}{}### TESTING ERRORS ###{}".format(C_B_WHITE, B_GRAY, RESET))
    run_test("Test 400: GET /iamnothere/", test_errors, "iamnothere/", 400)
    run_test("Test 404: GET /iamnothere.html", test_errors, "iamnothere.html", 404)

if __name__ == "__main__":
    cleanup()
    run()