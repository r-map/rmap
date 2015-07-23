# -*- coding: utf-8 -*-

# modified by Paolo Patruno September 2009

## Copyright 1999-2009 by LivingLogic AG, Bayreuth/Germany
## Copyright 1999-2009 by Walter Dörwald
##
## All Rights Reserved
##
## Permission is hereby granted, free of charge, to any person obtaining a copy
## of this software and associated documentation files (the "Software"), to deal
## in the Software without restriction, including without limitation the rights
## to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
## copies of the Software, and to permit persons to whom the Software is
## furnished to do so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in
## all copies or substantial portions of the Software.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
## THE SOFTWARE.
##
##
## This software includes spark by John Aycock
## http://pages.cpsc.ucalgary.ca/~aycock/spark/
##
##  Copyright (c) 1998-2002 John Aycock
##  
##  Permission is hereby granted, free of charge, to any person obtaining
##  a copy of this software and associated documentation files (the
##  "Software"), to deal in the Software without restriction, including
##  without limitation the rights to use, copy, modify, merge, publish,
##  distribute, sublicense, and/or sell copies of the Software, and to
##  permit persons to whom the Software is furnished to do so, subject to
##  the following conditions:
##  
##  The above copyright notice and this permission notice shall be
##  included in all copies or substantial portions of the Software.
##  
##  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
##  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
##  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
##  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
##  CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
##  TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
##  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
##
##
## This software includes sgmlop by Fredrik Lundh
## http://effbot.org/zone/sgmlop-index.htm
##
## Copyright (c) 1998-2007 by Secret Labs AB
## Copyright (c) 1998-2007 by Fredrik Lundh
##
## fredrik@pythonware.com
## http://www.pythonware.com
##
## By obtaining, using, and/or copying this software and/or its
## associated documentation, you agree that you have read, understood,
## and will comply with the following terms and conditions:
##
## Permission to use, copy, modify, and distribute this software and its
## associated documentation for any purpose and without fee is hereby
## granted, provided that the above copyright notice appears in all
## copies, and that both that copyright notice and this permission notice
## appear in supporting documentation, and that the name of Secret Labs
## AB or the author not be used in advertising or publicity pertaining to
## distribution of the software without specific, written prior
## permission.
##
## SECRET LABS AB AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO
## THIS SOFTWARE, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
## FITNESS.  IN NO EVENT SHALL SECRET LABS AB OR THE AUTHOR BE LIABLE FOR
## ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
## WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
## ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
## OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
##
##
## This software includes jsmin by Douglas Crockford/Baruch Even
## http://www.crockford.com/javascript/jsmin.py.txt
##
## This code is original from jsmin by Douglas Crockford, it was translated to
## Python by Baruch Even. The original code had the following copyright and
## license.
##
## /* jsmin.c
##    2007-05-22
##
## Copyright (c) 2002 Douglas Crockford  (www.crockford.com)
##
## Permission is hereby granted, free of charge, to any person obtaining a copy of
## this software and associated documentation files (the "Software"), to deal in
## the Software without restriction, including without limitation the rights to
## use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
## of the Software, and to permit persons to whom the Software is furnished to do
## so, subject to the following conditions:
##
## The above copyright notice and this permission notice shall be included in all
## copies or substantial portions of the Software.
##
## The Software shall be used for Good, not Evil.
##
## THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
## IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
## FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
## AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
## LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
## OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
## SOFTWARE.

ur"""
This module can be used on UNIX to fork a daemon process. It is based on
`Jürgen Hermann's Cookbook recipe`__.

__ http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/66012

An example script might look like this::


import daemon

tmp = daemon.Daemon(
        stdin="/dev/null",
        stdout="/tmp/tmp.log",
        stderr="/tmp/tmp.err",
        pidfile="/tmp/tmp.lock",
#        user=user,
#        group=group,
#        env=env
)


def main(self):

    import subprocess
    self.procs=[subprocess.Popen(["sleep","60"],cwd=self.cwd)]
    self.procs.append(subprocess.Popen(["sleep","130"],cwd=self.cwd))
    
if __name__ == '__main__':

    import sys, os

    tmp.cwd=os.getcwd()

    if tmp.service():

        sys.stdout.write("Daemon started with pid %d\n" % os.getpid())
        sys.stdout.write("Daemon stdout output\n")
        sys.stderr.write("Daemon stderr output\n")

        main(tmp)  # (this code was run as script)

        for proc in tmp.procs:
            proc.wait()

        sys.exit(0)
    

"""


import sys, os, signal, pwd, grp, optparse


__docformat__ = "reStructuredText"


class Daemon(object):
	"""
	The :class:`Daemon` class provides methods for starting and stopping a
	daemon process as well as handling command line arguments.
	"""
	def __init__(self, stdin="/dev/null", stdout="/dev/null", stderr="/dev/null", pidfile=None, user=None, group=None,env=None):
		"""
		The :var:`stdin`, :var:`stdout`, and :var:`stderr` arguments are file
		names that will be opened and be used to replace the standard file
		descriptors in ``sys.stdin``, ``sys.stdout``, and ``sys.stderr``.
		These arguments are optional and default to ``"/dev/null"``. Note that
		stderr is opened unbuffered, so if it shares a file with stdout then
		interleaved output may not appear in the order that you expect.

		:var:`pidfile` must be the name of a file. :meth:`start` will write
		the pid of the newly forked daemon to this file. :meth:`stop` uses this
		file to kill the daemon.

		:var:`user` can be the name or uid of a user. :meth:`start` will switch
		to this user for running the service. If :var:`user` is :const:`None` no
		user switching will be done.

		In the same way :var:`group` can be the name or gid of a group.
		:meth:`start` will switch to this group.

		:env: {} set the ENVIROMENT variables 

		"""
		options = dict(
			stdin=stdin,
			stdout=stdout,
			stderr=stderr,
			pidfile=pidfile,
			user=user,
			group=group
		)

		self.env=env
		self.options = optparse.Values(options)
		self.procs=()
		self.cwd=os.getcwd()


	def openstreams(self):
		"""
		Open the standard file descriptors stdin, stdout and stderr as specified
		in the constructor.
		"""
		si = open(self.options.stdin, "r")
		so = open(self.options.stdout, "a+")
		se = open(self.options.stderr, "a+", 0)
		os.dup2(si.fileno(), sys.stdin.fileno())
		os.dup2(so.fileno(), sys.stdout.fileno())
		os.dup2(se.fileno(), sys.stderr.fileno())
	
	def handlesighup(self, signum, frame):
		"""
		Handle a ``SIG_HUP`` signal: Reopen standard file descriptors.
		"""
		import subprocess

		self.openstreams()

		for proc in self.procs:
			if (isinstance(proc,subprocess.Popen)):
				#proc.send_signal(signum)     # work in py ver 2.6
				os.kill(proc.pid,signum)

			if (isistance(proc,int)):
				os.kill(proc,signum)

	def handlesigterm(self, signum, frame):
		"""
		Handle a ``SIG_TERM`` signal: Remove the pid file and exit.
		"""
		import subprocess

		if self.options.pidfile is not None:
			try:
				os.remove(self.options.pidfile)
			except Exception:
				pass

		for proc in self.procs:
			if (isinstance(proc,subprocess.Popen)):
				#proc.send_signal(signum)     # work in py ver 2.6
				os.kill(proc.pid,signum)

			if (isinstance(proc,int)):
				os.kill(proc,signum)

		sys.exit(0)

	def switchuser(self, user, group, env):
		"""
		Switch the effective user and group. If :var:`user` and :var:`group` are
		both :const:`None` nothing will be done. :var:`user` and :var:`group`
		can be an :class:`int` (i.e. a user/group id) or :class:`str`
		(a user/group name).
		"""
		groups = []
		if group is not None:
			if isinstance(group, list):
				for gr in group:
					if isinstance(gr, basestring):
						groups.append(grp.getgrnam(gr).gr_gid)
				group = group[0]

			if isinstance(group, basestring):
				group = grp.getgrnam(group).gr_gid

			try:
				os.setgroups(groups)
			except:
				pass

			os.setgid(group)
			os.setegid(group)

		if user is not None:
			if isinstance(user, basestring):
				user = pwd.getpwnam(user).pw_uid
			os.setuid(user)
			os.seteuid(user)

			# todo: check why home is set here
			#if not "HOME" in os.environ:
			os.environ["HOME"] = pwd.getpwuid(user).pw_dir

		if env is not None:
			for variable in env:
				os.environ[variable] = env[variable]

		if "HOME" in os.environ:
			self.cwd=os.environ["HOME"]

		try:
			os.chdir(self.cwd)
		except:
			pass

	def start(self):
		"""
		Daemonize the running script. When this method returns the process is
		completely decoupled from the parent environment.
		"""
		# Finish up with the current stdout/stderr
		sys.stdout.flush()
		sys.stderr.flush()
	
		# Do first fork
		try:
			pid = os.fork()
			if pid > 0:
				sys.exit(0) # Exit first parent
		except OSError, exc:
			sys.exit("%s: fork #1 failed: (%d) %s\n" % (sys.argv[0], exc.errno, exc.strerror))
	
		# Decouple from parent environment
		os.chdir("/")
		os.umask(0)
		os.setsid()
	
		# Do second fork
		try:
			pid = os.fork()
			if pid > 0:
				sys.exit(0) # Exit second parent
		except OSError, exc:
			sys.exit("%s: fork #2 failed: (%d) %s\n" % (sys.argv[0], exc.errno, exc.strerror))
	
		# Now I am a daemon!
	
		# Switch user
		self.switchuser(self.options.user, self.options.group, self.env)

		# Redirect standard file descriptors (will belong to the new user)
		self.openstreams()
	
		# Write pid file (will belong to the new user)
		if self.options.pidfile is not None:
			open(self.options.pidfile, "wb").write(str(os.getpid()))

		# Reopen file descriptors on SIGHUP
		signal.signal(signal.SIGHUP, self.handlesighup)

		# Remove pid file and exit on SIGTERM
		signal.signal(signal.SIGTERM, self.handlesigterm)

	def stop(self):
		"""
		Send a ``SIGTERM`` signal to a running daemon. The pid of the daemon
		will be read from the pidfile specified in the constructor.
		"""
		if self.options.pidfile is None:
			sys.exit("no pidfile specified")
		try:
			pidfile = open(self.options.pidfile, "rb")
		except IOError, exc:
			sys.exit("can't open pidfile %s: %s" % (self.options.pidfile, str(exc)))
		data = pidfile.read()
		try:
			pid = int(data)
		except ValueError:
			sys.exit("mangled pidfile %s: %r" % (self.options.pidfile, data))
		os.kill(pid, signal.SIGTERM)

	def optionparser(self):
		"""
		Return an :mod:`optparse` parser for parsing the command line options.
		This can be overwritten in subclasses to add more options.
		"""

		from . import  __version__

		p = optparse.OptionParser(usage="usage: %prog <action> [options] (action=start|stop|restart|run|version)",
					  description="%prog daemon for rmap suite",version="%prog "+__version__)
		p.add_option("--pidfile", dest="pidfile", help="PID filename (default %default)", default=self.options.pidfile)
		p.add_option("--stdin", dest="stdin", help="stdin filename (default %default)", default=self.options.stdin)
		p.add_option("--stdout", dest="stdout", help="stdout filename (default %default)", default=self.options.stdout)
		p.add_option("--stderr", dest="stderr", help="stderr filename (default %default)", default=self.options.stderr)
		p.add_option("--user", dest="user", help="user name or id (default %default)", default=self.options.user)
		p.add_option("--group", dest="group", help="group name or id (default %default)", default=self.options.group)
		return p

	def service(self, args=None,noptions=0):
		"""
		Handle command line arguments and start or stop the daemon accordingly.

		:var:`args` must be a list of command line arguments (including the
		program name in ``args[0]``). If :var:`args` is :const`None` or
		unspecified ``sys.argv`` is used.

		:var:`noptions` max number of options in command line or args

		The return value is true when a starting option has been specified as the
		command line argument, i.e. if the daemon should be started.

		The :mod:`optparse` options and arguments are available
		afterwards as ``self.options`` and ``self.args``.
		"""
		p = self.optionparser()
		if args is None:
			args = sys.argv
		(self.options, self.args) = p.parse_args(args)
		if len(self.args) == 1 or len(self.args) > noptions+2:
			p.error("incorrect number of arguments")
			sys.exit(1)
		if self.args[1] == "run":
			return True
		elif self.args[1] == "restart":
			try:
				self.stop()
			finally:
				self.start()
				return True
		elif self.args[1] == "start":
			self.start()
			return True
		elif self.args[1] == "stop":
			self.stop()
			return False
		else:
			p.error("incorrect argument %s" % self.args[1])
			sys.exit(1)
