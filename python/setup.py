from distutils.core import setup
import os,errno,sys

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

try:
    from rmap import __version__

    os.environ['DJANGO_SETTINGS_MODULE'] = 'rmap.settings'
    from django.conf import settings
    import django
    django.setup()

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
        try:
            shutil.rmtree("static")
        except:
            pass
        try:
            shutil.rmtree("build")
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
    sub_commands.append(('djangocollectstatic', None))
    sub_commands.append(('compilemessages', None))
    sub_commands.append(('createmanpages', None))

    
class  install(install_):
     def  run(self):
         from django.core import management
         #print "eseguo compilemessages"
         management.call_command("compilemessages")
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
            subprocess.check_call(["help2man","-N","-o","man/man1/poweroffd.1","./poweroffd"])
            subprocess.check_call(["gzip", "-f","man/man1/poweroffd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/composereportd.1","./composereportd"])
            subprocess.check_call(["gzip", "-f","man/man1/composereportd.1"])
            #subprocess.check_call(["help2man","-N","-o","man/man1/rmapweb.1","./rmapweb"])
            #subprocess.check_call(["gzip", "-f","man/man1/rmapweb.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2amqp_identvalidationd.1","./amqp2amqp_identvalidationd"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2amqp_identvalidationd.1"])
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2amqp_json2bufrd.1","./amqp2amqp_json2bufrd"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2amqp_json2bufrd.1"])
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
            subprocess.check_call(["help2man","-N","-o","man/man1/amqp2djangod.1","./amqp2djangod"])
            subprocess.check_call(["gzip", "-f","man/man1/amqp2djangod.1"])
            subprocess.check_call(["help2man","--no-discard-stderr","-N","-o","man/man1/dballe2arkimet.1","./dballe2arkimet"])
            subprocess.check_call(["gzip", "-f","man/man1/dballe2arkimet.1"])

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

        print "execute django collectstatic files"

        from django.core import management
        management.call_command("collectstatic", verbosity=0, interactive=False)

        #os.environ.setdefault("DJANGO_SETTINGS_MODULE", "rmap.settings")
        #from django.core.management import execute_from_command_line
        #execute_from_command_line([ "execname",'collectstatic',"--noinput"])



# Compile the list of files available, because distutils doesn't have
# an easy way to do this.
rmap_package_data = []
amatyr_package_data = []
borinud_package_data = []
geoimage_package_data = []
graphite_dballe_package_data = []
http2mqtt_package_data = []
insertdata_package_data = []
registration_package_data = []
showdata_package_data = []
rainbo_package_data = []

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

for dirpath, dirnames, filenames in os.walk('geoid_heights'):
    # Ignore dirnames that start with '.'
    for i, dirname in enumerate(dirnames):
        if dirname.startswith('.'): del dirnames[i]
    if filenames:
        data_files.append(['share/rmap/'+dirpath, [os.path.join(dirpath, f) for f in filenames]])


try:
    if (os.path.isdir("/etc/rmap/")):
        os.mkdir('/etc/rmap/tmp')
        os.rmdir('/etc/rmap/tmp')
    else:
        os.mkdir('/etc/rmap')
        os.rmdir('/etc/rmap')

    data_files.append(('/etc/rmap',['rmap-site.cfg']))
    data_files.append(('/etc/rmap',['map']))
    data_files.append(('/etc/rmap',['dashboard.conf']))
    data_files.append(('/etc/rmap',['graphTemplates.conf']))

except OSError as e:
    if (e[0] == errno.EACCES):
       print >> sys.stderr, "You do not have root permissions to install files in /etc !"
    else:
        print >> sys.stderr, "There are some problems to install files in /etc !"

for dirpath, dirnames, filenames in os.walk('rmap/static'):
    if filenames:
        for file in filenames:
            rmap_package_data.append( os.path.relpath(os.path.join(dirpath, file),'rmap'))
for dirpath, dirnames, filenames in os.walk('amatyr/static'):
    if filenames:
        for file in filenames:
            amatyr_package_data.append( os.path.relpath(os.path.join(dirpath, file),'amatyr'))
for dirpath, dirnames, filenames in os.walk('borinud/static'):
    if filenames:
        for file in filenames:
            borinud_package_data.append( os.path.relpath(os.path.join(dirpath, file),'borinud'))
for dirpath, dirnames, filenames in os.walk('geoimage/static'):
    if filenames:
        for file in filenames:
            geoimage_package_data.append( os.path.relpath(os.path.join(dirpath, file),'geoimage'))
for dirpath, dirnames, filenames in os.walk('graphite-dballe/static'):
    if filenames:
        for file in filenames:
            graphite_dballe_package_data.append( os.path.relpath(os.path.join(dirpath, file),'graphite-dballe'))
for dirpath, dirnames, filenames in os.walk('http2mqtt/static'):
    if filenames:
        for file in filenames:
            http2mqtt_package_data.append( os.path.relpath(os.path.join(dirpath, file),'http2mqtt'))
for dirpath, dirnames, filenames in os.walk('insertdata/static'):
    if filenames:
        for file in filenames:
            insertdata_package_data.append( os.path.relpath(os.path.join(dirpath, file),'insertdata'))
                    
for dirpath, dirnames, filenames in os.walk('registration/locale'):
    if filenames:
        for file in filenames:
            registration_package_data.append( os.path.relpath(os.path.join(dirpath, file),'registration'))
for dirpath, dirnames, filenames in os.walk('registration/static'):
    if filenames:
        for file in filenames:
            registration_package_data.append( os.path.relpath(os.path.join(dirpath, file),'registration'))

for dirpath, dirnames, filenames in os.walk('showdata/static'):
    if filenames:
        for file in filenames:
            showdata_package_data.append( os.path.relpath(os.path.join(dirpath, file),'showdata'))

for dirpath, dirnames, filenames in os.walk('rainbo/static'):
    if filenames:
        for file in filenames:
            rainbo_package_data.append( os.path.relpath(os.path.join(dirpath, file),'rainbo'))
            

#package_data.append('rmap_config')
#package_data.append('settings')

setup(name='rmap',
      version=__version__,
      description='rete monitoraggio ambientale partecipativo',
      long_description=README,
      author='Paolo Patruno',
      author_email='p.patruno@iperbole.bologna.it',
      platforms = ["any"],
      url='https://github.com/r-map/rmap',
      cmdclass={'build': build,'compilemessages':compilemessages,'makemessages':makemessages,'createmanpages':createmanpages,"distclean":distclean,'install': install,'djangocollectstatic':djangocollectstatic},
#      include_package_data=True,
#      packages=find_packages(),
      packages=['rmap','rmap.stations','rmap.stations.migrations','rmap.doc',
                'mapview',
                'http2mqtt',
                'registration','registration.management','registration.backends','registration.backends.default','registration.backends.simple','registration.management.commands','registration.migrations',
                'paho','paho.mqtt',
                'borinud','borinud.v1','borinud.utils','borinud.migrations',
                'geoimage','geoimage.migrations',
                'insertdata',
                'rmap.piexif',
                'amatyr',
                'showdata','showdata.migrations',
                'rainbo',
                'borinud_sos',
                'graphite-dballe','graphite-dballe.migrations',
                'graphite-dballe.account','graphite-dballe.account.migrations',
                'graphite-dballe.browser',
                'graphite-dballe.composer',
                'graphite-dballe.dashboard','graphite-dballe.dashboard.migrations',
                'graphite-dballe.events','graphite-dballe.events.migrations',
                'graphite-dballe.finders',
                'graphite-dballe.metrics',
                'graphite-dballe.render',
                'graphite-dballe.url_shortener','graphite-dballe.url_shortener.migrations',
                'graphite-dballe.version',
                'graphite-dballe.whitelist',
      ],

      package_data={
          'rmap': ['icons/*.png','tables/*.txt','templates/*.htm*','templates/*.txt','templates/stations/*']+rmap_package_data,
          'rmap.stations': ['fixtures/*.json'],
          'mapview': ['icons/*.png'],          
          'amatyr':['templates/amatyr/*']+amatyr_package_data,
          'borinud':['templates/borinud/*']+borinud_package_data,
          'geoimage':['templates/geoimage/*']+geoimage_package_data,
          'graphite-dballe':['templates/*']+graphite_dballe_package_data,
          'http2mqtt':['templates/*']+http2mqtt_package_data,
          'insertdata':['templates/insertdata/*']+insertdata_package_data,
          'registration':['templates/registration/*']+registration_package_data,
          'showdata':['templates/showdata/*']+showdata_package_data,
          'rainbo':['templates/rainbo/*.html','templates/rainbo/base_service/*.html']+rainbo_package_data,          
          'borinud_sos':['templates/borinud_sos/xml/1.0/*.xml'],          
      },
      scripts=[
          'stationd','mqtt2graphited','mqtt2dballed','poweroffd','composereportd','rmapweb','amqp2amqp_identvalidationd',
          'amqp2amqp_json2bufrd','amqp2dballed', 'amqp2arkimetd','amqp2mqttd','rmap-configure','rmapctrl','rmap.wsgi',
          'rmapgui','amqp2djangod','amqp2geoimaged','dballe2arkimet'],
      data_files = data_files,
      license = "GNU GPL v2",
      install_requires= [ 'Django>=1.9,<1.9.99',"configobj","plyer","pika","simplejson","futures","requests","pyserial","django-leaflet","django-jsonfield","django-geojson","Pillow","django-imagekit","django-appconf","nominatim","django-hosts","iso8601"],
      extras_require = {
          'borinud': ['dballe', 'django-tagging==0.4.3', 'pytz', 'pyparsing==1.5.7', 'cairocffi',
                      'django-classy-tags','django_cookie_law']
      },
      #install_requires= [ "django","Cython","pil","pysdl2","kivy","plyer","configobj","pika","simplejson"],
      #setup_requires= [ "django","configobj"],
     )

