from distutils.core import setup
import os,errno,sys
from sys import platform

from distutils.command.build import build as build_
from setuptools.command.develop import develop as develop_
from setuptools.command.install import install as install_
from distutils.core import Command
#from buildutils.cmd import Command
#from distutils.cmd import Command

with open(os.path.join(os.path.dirname(__file__), 'README.rst')) as readme:
    README = readme.read()

# allow setup.py to be run from any path
os.chdir(os.path.normpath(os.path.join(os.path.abspath(__file__), os.pardir)))


class distclean(Command):
    description = "remove man pages and *.mo files"
    user_options = []
    boolean_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        import shutil
        from os.path import join
        try:
            print('removing: man dir')
            if not(self.dry_run): shutil.rmtree("man")
        except:
            pass
        try:
            print('removing: build dir')
            if not(self.dry_run): shutil.rmtree("build")
        except:
            pass

        for root, dirs, files in os.walk('rpc/locale'):
            for name in files:
                if name[-3:] == ".mo":
                    print('removing: %s' % join(root, name))
                    if not(self.dry_run): os.remove(join(root, name))

        # remove all the .pyc files
        for root, dirs, files in os.walk(os.getcwd(), topdown=False):
            for name in files:
                if name.endswith('.pyc') and os.path.isfile(join(root, name)):
                    print('removing: %s' % join(root, name))
                    if not(self.dry_run): os.remove(join(root, name))


class build(build_):

    sub_commands = build_.sub_commands[:]
    sub_commands.append(('createmanpages', None))

    
class  install(install_):
     def  run(self):
         from django.core import management
         #print "eseguo compilemessages"
         #management.call_command("compilemessages")
         #management.call_command("collectstatic")
         #createmanpages().run()
         install_.run(self)

class makemessages(Command):
    description = "Runs over the entire source tree of the current directory and pulls out all strings marked for translation."
    user_options = []
    boolean_options = []
    
    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):

        # rm build to do not duplicate messages
        try:
            shutil.rmtree("build")
        except:
            pass

        from django.core import management
        management.call_command("makemessages",all=True)


class createmanpages(Command):
    description = "generate man page with help2man"
    user_options = []
    boolean_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        try:
            import subprocess
            subprocess.check_call(["mkdir","-p", "man/man1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/rpcd.1","./rpcd"])
            subprocess.check_call(["gzip", "-f","man/man1/rpcd.1"])

        except:
            pass


class djangocollectstatic(Command):
    description = "collect static files for web server to serve it"
    user_options = []   
    boolean_options = []

    def initialize_options(self):
        pass
    
    def finalize_options(self):
        pass

    def run(self):

        print("execute django collectstatic files")

        from django.core import management
        management.call_command("collectstatic", verbosity=0, interactive=False)


# Compile the list of files available, because distutils doesn't have
# an easy way to do this.
rpc_package_data = []
data_files = []

for dirpath, dirnames, filenames in os.walk('man'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])


for dirpath, dirnames, filenames in os.walk('static'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])


for dirpath, dirnames, filenames in os.walk('doc'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])

for dirpath, dirnames, filenames in os.walk('rpc/locale'):
    if filenames:
        for file in filenames:
            rpc_package_data.append( os.path.relpath(os.path.join(dirpath, file),'rpc'))    
for dirpath, dirnames, filenames in os.walk('rpc/static'):
    if filenames:
        for file in filenames:
            rpc_package_data.append( os.path.relpath(os.path.join(dirpath, file),'rpc'))
setup(name='rmap-rpc',
      version="16.2",
      description='rete monitoraggio ambientale partecipativo - RPC subpackage',
      long_description=README,
      author='Paolo Patruno',
      author_email='p.patruno@iperbole.bologna.it',
      platforms = ["any"],
      url='https://github.com/r-map/rmap',
      cmdclass={'build': build,'makemessages':makemessages,'createmanpages':createmanpages,"distclean":distclean,'install': install,'djangocollectstatic':djangocollectstatic},
#      include_package_data=True,
#      packages=find_packages(),
      packages=['rpc','rpc.migrations',
      ],

      package_data={
          'rpc': ['templates/*.htm*','templates/*.txt'],
          'rpc': ['fixtures/*.json'],
      },
      scripts=['rpcd',],
      data_files = data_files,
      license = "GNU GPL v2",
      install_requires= [ 'rmap','django>=2.0,<3.0',"simplejson","paho-mqtt",]
      )
