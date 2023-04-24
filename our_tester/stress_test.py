#!/usr/bin/python3
import threading
import tests
import subprocess

""" Function to run subprocesses """
def run_subprocess() -> None:
	subprocess.run(["our_tester/main.py"])

""" create different threads to send requests to the server """
def stress_test() -> str:
	threads = []
	for i in range(50):
		t = threading.Thread(target=run_subprocess)
		threads.append(t)
		t.start()
	for t in threads:
		t.join()
	return ""

def main() -> None:
	stress_test()

main()