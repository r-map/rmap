from distutils.core import setup
import os,errno,sys

with open(os.path.join(os.path.dirname(__file__), 'README.rst')) as readme:
    README = readme.read()

# allow setup.py to be run from any path
os.chdir(os.path.normpath(os.path.join(os.path.abspath(__file__), os.pardir)))

from borinud import __version__

# Compile the list of files available, because distutils doesn't have
# an easy way to do this.
borinud_package_data = []

data_files = []

for dirpath, dirnames, filenames in os.walk('borinud/static'):
    if filenames:
        for file in filenames:
            borinud_package_data.append( os.path.relpath(os.path.join(dirpath, file),'borinud'))

setup(name='django-borinud',
      version=__version__,
      description='borinud django app',
      long_description=README,
      author='Paolo Patruno',
      author_email='p.patruno@iperbole.bologna.it',
      platforms = ["any"],
      url='https://github.com/r-map/rmap',
#      include_package_data=True,
#      packages=find_packages(),
      packages=['borinud','borinud.v1','borinud.utils','borinud.migrations'],
      package_data={'borinud':['templates/borinud/*']+borinud_package_data},
      data_files = data_files,
      license = "GNU GPL v2",
      install_requires= [ 'django>=2.0,<3.0',"configobj","pika","simplejson","requests","pyserial","django-leaflet","jsonfield","django-geojson","Pillow","django-imagekit","django-appconf","nominatim","django-hosts","iso8601","django-cookie-law","django-tagging","pytz","six","paho-mqtt"],
      extras_require = {
          'borinud': ['dballe>=8.4', 'django-tagging==0.4.3', 'pytz', 'pyparsing==1.5.7', 'cairocffi',
                      'classytags','cookielaw','numpy']
      },
     )

