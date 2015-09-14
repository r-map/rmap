from distutils.core import setup
import os,errno,sys

from distutils.command.build import build as build_
from setuptools.command.develop import develop as develop_
from setuptools.command.install import install as install_
from distutils.core import Command
#from buildutils.cmd import Command
#from distutils.cmd import Command

try:
    from rmap import __version__

    os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
    from django.conf import settings

except:
    print "error setting django env"

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
            shutil.rmtree("man")
        except:
            pass
        for root, dirs, files in os.walk('locale'):
            for name in files:
                if name[-3:] == ".mo":
                    os.remove(join(root, name))

        # remove all the .pyc files
        for root, dirs, files in os.walk(os.getcwd(), topdown=False):
            for name in files:
                if name.endswith('.pyc') and os.path.isfile(os.path.join(root, name)):
                    print 'removing: %s' % os.path.join(root, name)
                    if not(self.dry_run): os.remove(os.path.join(root, name))


class build(build_):

    sub_commands = build_.sub_commands[:]
    sub_commands.append(('compilemessages', None))
    sub_commands.append(('createmanpages', None))


class  install(install_):
     def  run(self):
         from django.core import management
         #print "eseguo compilemessages"
         management.call_command("compilemessages")
         #createmanpages().run()
         install_.run(self)


class compilemessages(Command):
    description = "generate .mo files from .po"
    user_options = []
    boolean_options = []

    def initialize_options(self):
        pass

    def finalize_options(self):
        pass

    def run(self):
        from django.core import management
        management.call_command("compilemessages")

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
            subprocess.check_call(["help2man","-N","-o","man/man1/stationd.1","./stationd"])
            subprocess.check_call(["gzip","-f", "man/man1/stationd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/mqtt2graphited.1","./mqtt2graphited"])
            subprocess.check_call(["gzip", "-f","man/man1/mqtt2graphited.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/mqtt2dballed.1","./mqtt2dballed"])
            subprocess.check_call(["gzip", "-f","man/man1/mqtt2dballed.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/borinudd.1","./borinudd"])
            subprocess.check_call(["gzip", "-f","man/man1/borinudd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/poweroffd.1","./poweroffd"])
            subprocess.check_call(["gzip", "-f","man/man1/poweroffd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/composereportd.1","./composereportd"])
            subprocess.check_call(["gzip", "-f","man/man1/composereportd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/rmapweb.1","./rmapweb"])
            subprocess.check_call(["gzip", "-f","man/man1/rmapweb.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2amqp_identvalidationd.1","./amqp2amqp_identvalidationd"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2amqp_identvalidationd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2dballed.1","./amqp2dballed"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2dballed.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2arkimetd.1","./amqp2arkimetd"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2arkimetd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2mqttd.1","./amqp2mqttd"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2mqttd.1"])
            subprocess.check_call(["help2man","-N","--no-discard-stderr","-o","man/man1/rmap-configure.1","./rmap-configure"])
            subprocess.check_call(["gzip", "-f","man/man1/rmap-configure.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/rmapctrl.1","./rmapctrl"])
            subprocess.check_call(["gzip", "-f","man/man1/rmapctrl.1"])

        except:
            pass

# Compile the list of files available, because distutils doesn't have
# an easy way to do this.
package_data = []
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

for dirpath, dirnames, filenames in os.walk('locale'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])

for dirpath, dirnames, filenames in os.walk('templates'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])

for dirpath, dirnames, filenames in os.walk('geoid_heights'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])


#for dirpath, dirnames, filenames in os.walk('tables'):
#    # Ignore dirnames that start with '.'
#    for i, dirname in enumerate(dirnames):
#        if dirname.startswith('.'): del dirnames[i]
#    if filenames:
#        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])


try:
    if (os.path.isdir("/etc/rmap/")):
        os.mkdir('/etc/rmap/tmp')
        os.rmdir('/etc/rmap/tmp')
    else:
        os.mkdir('/etc/rmap')
        os.rmdir('/etc/rmap')

    data_files.append(('/etc/rmap',['rmap-site.cfg']))
    data_files.append(('/etc/rmap',['map']))

except OSError as e:
    if (e[0] == errno.EACCES):
       print >> sys.stderr, "You do not have root permissions to install files in /etc !"
    else:
        print >> sys.stderr, "There are some problems to install files in /etc !"

#for dirpath, dirnames, filenames in os.walk('rmap/templates'):
#    # Ignore dirnames that start with '.'
#    for i, dirname in enumerate(dirnames):
#        if dirname.startswith('.'): del dirnames[i]
#    if filenames:
#        for file in filenames:
#            package_data.append('templates/'+ os.path.join(dirname, file))
#
#for dirpath, dirnames, filenames in os.walk('rmap/locale'):
#    # Ignore dirnames that start with '.'
#    for i, dirname in enumerate(dirnames):
#        if dirname.startswith('.'): del dirnames[i]
#    if filenames:
#        for file in filenames:
#            package_data.append('locale/'+ os.path.join(dirname, file))

#package_data.append('rmap_config')
#package_data.append('settings')

setup(name='rmap',
      version=__version__,
      description='rete monitoraggio ambientale partecipativo',
      author='Paolo Patruno',
      author_email='p.patruno@iperbole.bologna.it',
      platforms = ["any"],
      url='https://github.com/r-map/rmap',
      cmdclass={'build': build,'compilemessages':compilemessages,'createmanpages':createmanpages,"distclean":distclean,'install': install},
      packages=['rmap','rmap.stations','rmap.stations.migrations','rmap.doc','mapview','registration','registration.management','registration.backends','registration.backends.default','registration.backends.simple','registration.management.commands','paho','paho.mqtt'],
      package_data={'rmap': ['icons/*.png','tables/*.txt'],'rmap.stations': ['fixtures/*.json'],'mapview': ['icons/*.png'],},
      scripts=['stationd','mqtt2graphited','borinudd','mqtt2dballed','poweroffd','composereportd','rmapweb','amqp2amqp_identvalidationd','amqp2dballed', 'amqp2arkimetd','amqp2mqttd','rmap-configure','rmapctrl','rmap.wsgi','rmapgui'],
      data_files = data_files,
      license = "GNU GPL v2",
      install_requires= [ "django","configobj","plyer","pika","simplejson","futures","requests","pyserial"],
      #install_requires= [ "django","Cython","pil","pysdl2","kivy","plyer","configobj","pika","simplejson"],
      #setup_requires= [ "django","configobj"],
      long_description="""\
R-map: participative environmental monitoring net.
"""
     )

