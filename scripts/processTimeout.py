import threading
import shlex
import subprocess

class WorkerThread(threading.Thread):
	def __init__(self, timeout, cmd):
		super(WorkerThread, self).__init__()
		self.timeout = timeout
		self.cmd     = shlex.split(cmd)

	def run(self):
		self.process = subprocess.Popen(self.cmd, stdout=subprocess.PIPE)
		output = self.process.communicate()
		self.output = output[0], self.process.returncode

	def start(self):
		threading.Thread.start(self)
		threading.Thread.join(self, self.timeout)
		if self.isAlive():
			self.process.terminate()
			return ["Process timeout", 0]
		return self.output
