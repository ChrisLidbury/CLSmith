# From http://stackoverflow.com/questions/1191374/subprocess-with-timeout
# and http://stackoverflow.com/questions/19959271/in-python-how-can-i-use-timeout-in-subprocess-and-get-the-output

import subprocess
import threading


class ThreadWithReturnValue(threading.Thread):
    def __init__(self, group=None, target=None, name=None, args=(), kwargs={}, Verbose=None):
        threading.Thread.__init__(self, group, target, name, args, kwargs, Verbose)
        self._return = None

    def run(self):
        if self._Thread__target is not None:
            self._return = self._Thread__target(*self._Thread__args, **self._Thread__kwargs)

    def join(self, timeout = None):
        threading.Thread.join(self, timeout)
        return self._return

class Command(object):
    def __init__(self, cmd):
        self.cmd = cmd
        self.process = None

    def run(self, timeout):
        def target(cmd):
            # print 'Thread started'
            self.process = subprocess.Popen(cmd, shell=True,  stdout=subprocess.PIPE, stderr = subprocess.PIPE)
            output = self.process.communicate()
            return output[0]
            # print 'Thread finished'

        thread = ThreadWithReturnValue(target=target, args=[self.cmd])
        thread.start()

        output = thread.join(timeout)
        if thread.is_alive():
            # print 'Terminating process'
            self.process.kill()
            thread.join()
            output = "Process timeout."
        return self.process.returncode, output
