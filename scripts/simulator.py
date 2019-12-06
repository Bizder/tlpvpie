import os
import subprocess


def run_simulator():
	process = subprocess.Popen('./waf --run pvpie-simulator', shell=True, stdin=subprocess.PIPE, stdout=subprocess.PIPE)
	out, err = process.communicate('5\n')

	print out


if __name__ == '__main__':
	cwd = os.getcwd()
	if cwd.endswith('scratch'):
		os.chdir(cwd.rsplit('/', 1)[0])

	run_simulator()

	os.chdir(cwd)
