# Django settings for rmap project.

import os
from configobj import ConfigObj,flatten_errors
from validate import Validator
from . import __version__
import imp

android=('ANDROID_ARGUMENT' in os.environ)

configspec={}

configspec['django']={}

configspec['django']['DEBUG']="boolean(default=True)"
configspec['django']['TEMPLATE_DEBUG']="boolean(default=True)"
configspec['django']['FILE_UPLOAD_PERMISSIONS']="integer(default=420)"
configspec['django']['SECRET_KEY']="string(default='random-string-of-ascii')"
configspec['django']['SESSION_COOKIE_DOMAIN']="string(default=None)"
configspec['django']['SERVER_EMAIL']="string(default='localhost')"
configspec['django']['EMAIL_HOST']="string(default='localhost')"
configspec['django']['EMAIL_PORT']="integer(default=25)"
configspec['django']['EMAIL_USE_TLS']="boolean(default=False)"
configspec['django']['EMAIL_HOST_USER']="string(default=None)"
configspec['django']['EMAIL_HOST_PASSWORD']="string(default=None)"
configspec['django']['DEFAULT_FROM_EMAIL']="string(default='admin@myrmap.cc')"
configspec['django']['ACCOUNT_ACTIVATION_DAYS']="integer(default=2)"
configspec['django']['REGISTRATION_AUTO_LOGIN']="boolean(default=False)"
configspec['django']['REGISTRATION_OPEN']="boolean(default=False)"
configspec['django']['REGISTRATION_EMAIL_SUBJECT_PREFIX']="string(default='RMAP:')"


#configspec['django']['TIME_ZONE']="string(default='Europe/Rome')"
configspec['django']['TIME_ZONE']="string(default='GMT')"
configspec['django']['LANGUAGE_CODE']="string(default='en-us')"
configspec['django']['SITE_ID']="integer(default=1)"
configspec['django']['USE_I18N']="boolean(default=True)"
configspec['django']['LOCALE_PATHS']="list(default=list('locale',))"
configspec['django']['ADMINS']="list(default=list('',))"
configspec['django']['MANAGERS']="list(default=list('',))"
configspec['django']['MEDIA_ROOT']="string(default='%s/media/')" % os.getcwd()
configspec['django']['MEDIA_SITE_ROOT']="string(default='%s/media/')" % os.getcwd()
configspec['django']['BASE_URL']="string(default='/django/')"
configspec['django']['ADMIN_MEDIA_PREFIX']="string(default='/django/media/admin/')"
configspec['django']['STATIC_URL']="string(default='/static/')"
configspec['django']['STATIC_ROOT'] = "string(default='%s/static/')" % os.getcwd()
configspec['django']['MEDIA_PREFIX']="string(default='/media/')"
configspec['django']['MEDIA_SITE_PREFIX']="string(default='/media/sito/')"
configspec['django']['SERVE_STATIC']="boolean(default=True)"


configspec['daemon']={}

configspec['daemon']['amqpuser']        = "string(default='rmap')"
configspec['daemon']['amqppassword']    = "string(default='rmap')"
configspec['daemon']['mqttuser']        = "string(default='rmap')"
configspec['daemon']['mqttpassword']    = "string(default='rmap')"


configspec['rmapweb']={}

configspec['rmapweb']['logfile']  = "string(default='/tmp/rmapweb.log')"
configspec['rmapweb']['graphiteinfologfile']  = "string(default='/tmp/graphiteinfo.log')"
configspec['rmapweb']['graphiteexceptionlogfile']  = "string(default='/tmp/graphiteexception.log')"
configspec['rmapweb']['graphitecachelogfile']  = "string(default='/tmp/graphitecache.log')"
configspec['rmapweb']['graphiterenderinglogfile']  = "string(default='/tmp/graphiterendering.log')"
configspec['rmapweb']['errfile']  = "string(default='/tmp/rmapweb.err')"
configspec['rmapweb']['lockfile'] = "string(default='/tmp/rmapweb.lock')"
configspec['rmapweb']['user']     = "string(default=None)"
configspec['rmapweb']['group']    = "string(default=None)"
configspec['rmapweb']['port']    = "string(default='127.0.0.1:8888')"


configspec['database']={}

configspec['database']['DATABASE_USER']="string(default='')"
configspec['database']['DATABASE_PASSWORD']="string(default='')"
configspec['database']['DATABASE_HOST']="string(default='localhost')"
configspec['database']['DATABASE_PORT']="integer(default=3306)"
configspec['database']['DATABASE_ENGINE']="string(default='sqlite3')"
configspec['database']['DATABASE_NAME']="string(default='%s/rmap.sqlite3')" % os.getcwd()


configspec['stationd']={}

configspec['stationd']['logfile']  = "string(default='/tmp/stationd.log')"
configspec['stationd']['errfile']  = "string(default='/tmp/stationd.err')"
configspec['stationd']['lockfile'] = "string(default='/tmp/stationd.lock')"
configspec['stationd']['conffile'] = "string(default='dbus-autoradio.conf')"
configspec['stationd']['user']     = "string(default=None)"
configspec['stationd']['group']    = "string(default=None)"
configspec['stationd']['stationslug']    = "string(default='home')"
configspec['stationd']['ident']          = "string(default=None)"
configspec['stationd']['boardslug']      = "string(default='base')"


configspec['mqtt2graphited']={}

configspec['mqtt2graphited']['logfile']  = "string(default='/tmp/mqtt2graphited.log')"
configspec['mqtt2graphited']['errfile']  = "string(default='/tmp/mqtt2graphited.err')"
configspec['mqtt2graphited']['lockfile'] = "string(default='/tmp/mqtt2graphited.lock')"
configspec['mqtt2graphited']['user']     = "string(default=None)"
configspec['mqtt2graphited']['group']    = "string(default=None)"
configspec['mqtt2graphited']['mapfile'] = "string(default='map')"

configspec['amqp2dballed']={}
configspec['amqp2dballed']['logfile']  = "string(default='/tmp/amqp2dballed.log')"
configspec['amqp2dballed']['errfile']  = "string(default='/tmp/amqp2dballed.err')"
configspec['amqp2dballed']['lockfile'] = "string(default='/tmp/amqp2dballed.lock')"
configspec['amqp2dballed']['user']     = "string(default=None)"
configspec['amqp2dballed']['group']    = "string(default=None)"
configspec['amqp2dballed']['dsn']      = "string(default='mysql:///report_fixed?user=rmap&password=rmap')"

configspec['amqp2amqp_identvalidationd']={}
configspec['amqp2amqp_identvalidationd']['logfile']  = "string(default='/tmp/amqp2amqp_identvalidationd.log')"
configspec['amqp2amqp_identvalidationd']['errfile']  = "string(default='/tmp/amqp2amqp_identvalidationd.err')"
configspec['amqp2amqp_identvalidationd']['lockfile'] = "string(default='/tmp/amqp2amqp_identvalidationd.lock')"
configspec['amqp2amqp_identvalidationd']['user']     = "string(default=None)"
configspec['amqp2amqp_identvalidationd']['group']    = "string(default=None)"

configspec['amqp2amqp_json2bufrd']={}
configspec['amqp2amqp_json2bufrd']['logfile']  = "string(default='/tmp/amqp2amqp_json2bufrd.log')"
configspec['amqp2amqp_json2bufrd']['errfile']  = "string(default='/tmp/amqp2amqp_json2bufrd.err')"
configspec['amqp2amqp_json2bufrd']['lockfile'] = "string(default='/tmp/amqp2amqp_json2bufrd.lock')"
configspec['amqp2amqp_json2bufrd']['user']     = "string(default=None)"
configspec['amqp2amqp_json2bufrd']['group']    = "string(default=None)"


configspec['amqp2djangod']={}
configspec['amqp2djangod']['logfile']  = "string(default='/tmp/amqp2django.log')"
configspec['amqp2djangod']['errfile']  = "string(default='/tmp/amqp2django.err')"
configspec['amqp2djangod']['lockfile'] = "string(default='/tmp/amqp2django.lock')"
configspec['amqp2djangod']['user']     = "string(default=None)"
configspec['amqp2djangod']['group']    = "string(default=None)"


configspec['amqp2geoimaged']={}
configspec['amqp2geoimaged']['logfile']  = "string(default='/tmp/amqp2geoimage.log')"
configspec['amqp2geoimaged']['errfile']  = "string(default='/tmp/amqp2geoimage.err')"
configspec['amqp2geoimaged']['lockfile'] = "string(default='/tmp/amqp2geoimage.lock')"
configspec['amqp2geoimaged']['user']     = "string(default=None)"
configspec['amqp2geoimaged']['group']    = "string(default=None)"


configspec['amqp2arkimetd']={}
configspec['amqp2arkimetd']['logfile']  = "string(default='/tmp/amqp2arkimetd.log')"
configspec['amqp2arkimetd']['errfile']  = "string(default='/tmp/amqp2arkimetd.err')"
configspec['amqp2arkimetd']['lockfile'] = "string(default='/tmp/amqp2arkimetd.lock')"
configspec['amqp2arkimetd']['user']     = "string(default=None)"
configspec['amqp2arkimetd']['group']    = "string(default=None)"

configspec['amqp2mqttd']={}
configspec['amqp2mqttd']['logfile']  = "string(default='/tmp/amqp2mqttd.log')"
configspec['amqp2mqttd']['errfile']  = "string(default='/tmp/amqp2mqttd.err')"
configspec['amqp2mqttd']['lockfile'] = "string(default='/tmp/amqp2mqttd.lock')"
configspec['amqp2mqttd']['user']     = "string(default=None)"
configspec['amqp2mqttd']['group']    = "string(default=None)"

configspec['mqtt2dballed']={}
configspec['mqtt2dballed']['logfile']   = "string(default='/tmp/mqtt2dballed.log')"
configspec['mqtt2dballed']['errfile']   = "string(default='/tmp/mqtt2dballed.err')"
configspec['mqtt2dballed']['lockfile']  = "string(default='/tmp/mqtt2dballed.lock')"
configspec['mqtt2dballed']['user']      = "string(default=None)"
configspec['mqtt2dballed']['group']     = "string(default=None)"
configspec['mqtt2dballed']['dsnsample_fixed']    = "string(default='mysql:///sample_fixed?user=rmap&password=rmap')"
configspec['mqtt2dballed']['dsnsample_mobile']   = "string(default='mysql:///sample_mobile?user=rmap&password=rmap')"
configspec['mqtt2dballed']['dsnreport_fixed']    = "string(default='mysql:///report_fixed?user=rmap&password=rmap')"
configspec['mqtt2dballed']['dsnreport_mobile']   = "string(default='mysql:///report_mobile?user=rmap&password=rmap')"
configspec['mqtt2dballed']['topicsample']   = "string(default='sample')"
configspec['mqtt2dballed']['topicreport']   = "string(default='report')"

configspec['composereportd']={}
configspec['composereportd']['logfile']  = "string(default='/tmp/composereportd.log')"
configspec['composereportd']['errfile']  = "string(default='/tmp/composereportd.err')"
configspec['composereportd']['lockfile'] = "string(default='/tmp/composereportd.lock')"
configspec['composereportd']['user']     = "string(default=None)"
configspec['composereportd']['group']    = "string(default=None)"
configspec['composereportd']['exchange']    = "string(default='rmap')"


config    = ConfigObj ('/etc/rmap/rmap-site.cfg',file_error=False,configspec=configspec)

usrconfig = ConfigObj (os.path.expanduser('~/.rmap.cfg'),file_error=False)
config.merge(usrconfig)
usrconfig = ConfigObj ('rmap.cfg',file_error=False)
config.merge(usrconfig)

val = Validator()
test = config.validate(val,preserve_errors=True)
for entry in flatten_errors(config, test):
    # each entry is a tuple
    section_list, key, error = entry
    if key is not None:
       section_list.append(key)
    else:
        section_list.append('[missing section]')
    section_string = ', '.join(section_list)
    if error == False:
        error = 'Missing value or section.'
    print section_string, ' = ', error
    raise error

# section django
DEBUG                   = config['django']['DEBUG']
FILE_UPLOAD_PERMISSIONS = config['django']['FILE_UPLOAD_PERMISSIONS']
SECRET_KEY              = config['django']['SECRET_KEY']
SESSION_COOKIE_DOMAIN   = config['django']['SESSION_COOKIE_DOMAIN']
SERVER_EMAIL            = config['django']['SERVER_EMAIL']
EMAIL_HOST              = config['django']['EMAIL_HOST']
EMAIL_PORT              = config['django']['EMAIL_PORT']         
EMAIL_USE_TLS           = config['django']['EMAIL_USE_TLS']      
EMAIL_HOST_USER         = config['django']['EMAIL_HOST_USER']    
EMAIL_HOST_PASSWORD     = config['django']['EMAIL_HOST_PASSWORD']
DEFAULT_FROM_EMAIL      = config['django']['DEFAULT_FROM_EMAIL']
ACCOUNT_ACTIVATION_DAYS = config['django']['ACCOUNT_ACTIVATION_DAYS']
REGISTRATION_AUTO_LOGIN = config['django']['REGISTRATION_AUTO_LOGIN']
REGISTRATION_OPEN       = config['django']['REGISTRATION_OPEN']
REGISTRATION_EMAIL_SUBJECT_PREFIX = config['django']['REGISTRATION_EMAIL_SUBJECT_PREFIX']


TIME_ZONE               = config['django']['TIME_ZONE']
LANGUAGE_CODE           = config['django']['LANGUAGE_CODE']
SITE_ID                 = config['django']['SITE_ID']
USE_I18N                = config['django']['USE_I18N']
LOCALE_PATHS            = config['django']['LOCALE_PATHS']
ADMINS                  = config['django']['ADMINS']
MANAGERS                = config['django']['MANAGERS']
MEDIA_ROOT              = config['django']['MEDIA_ROOT']
if "%s" in MEDIA_ROOT:
    MEDIA_ROOT = MEDIA_ROOT  % os.getcwd()
MEDIA_SITE_ROOT         = config['django']['MEDIA_SITE_ROOT']
if "%s" in MEDIA_SITE_ROOT:
    MEDIA_SITE_ROOT = MEDIA_SITE_ROOT  % os.getcwd()
BASE_URL                = config['django']['BASE_URL']
ADMIN_MEDIA_PREFIX      = config['django']['ADMIN_MEDIA_PREFIX']
STATIC_URL              = config['django']['STATIC_URL']
STATIC_ROOT             = config['django']['STATIC_ROOT']
if "%s" in STATIC_ROOT:
    STATIC_ROOT = STATIC_ROOT  % os.getcwd()
MEDIA_PREFIX            = config['django']['MEDIA_PREFIX']
MEDIA_SITE_PREFIX       = config['django']['MEDIA_SITE_PREFIX']
SERVE_STATIC            = config['django']['SERVE_STATIC']
MEDIA_URL               = "media/"
SITE_MEDIA_URL          = BASE_URL+MEDIA_SITE_PREFIX


# section daemon
amqpuser                = config['daemon']['amqpuser']
amqppassword            = config['daemon']['amqppassword']
mqttuser                = config['daemon']['mqttuser']
mqttpassword            = config['daemon']['mqttpassword']

# section rmapweb
logfileweb              = config['rmapweb']['logfile']
GRAPHITEINFOLOG         = config['rmapweb']['graphiteinfologfile']
GRAPHITEEXCEPTIONLOG    = config['rmapweb']['graphiteexceptionlogfile']
GRAPHITECACHELOG        = config['rmapweb']['graphitecachelogfile']
GRAPHITERENDERINGLOG    = config['rmapweb']['graphiterenderinglogfile']
errfileweb              = config['rmapweb']['errfile']
lockfileweb             = config['rmapweb']['lockfile']
userweb                 = config['rmapweb']['user']
groupweb                = config['rmapweb']['group']
port                    = config['rmapweb']['port']

# section amqp2dballed
logfileamqp2dballed              = config['amqp2dballed']['logfile']
errfileamqp2dballed              = config['amqp2dballed']['errfile']
lockfileamqp2dballed             = config['amqp2dballed']['lockfile']
useramqp2dballed                 = config['amqp2dballed']['user']
groupamqp2dballed                = config['amqp2dballed']['group']
dsn                              = config['amqp2dballed']['dsn']

# section amqp2amqp_identvalidationd
logfileamqp2amqp_identvalidationd              = config['amqp2amqp_identvalidationd']['logfile']
errfileamqp2amqp_identvalidationd              = config['amqp2amqp_identvalidationd']['errfile']
lockfileamqp2amqp_identvalidationd             = config['amqp2amqp_identvalidationd']['lockfile']
useramqp2amqp_identvalidationd                 = config['amqp2amqp_identvalidationd']['user']
groupamqp2amqp_identvalidationd                = config['amqp2amqp_identvalidationd']['group']

# section amqp2amqp_json2bufrd
logfileamqp2amqp_json2bufrd              = config['amqp2amqp_json2bufrd']['logfile']
errfileamqp2amqp_json2bufrd              = config['amqp2amqp_json2bufrd']['errfile']
lockfileamqp2amqp_json2bufrd             = config['amqp2amqp_json2bufrd']['lockfile']
useramqp2amqp_json2bufrd                 = config['amqp2amqp_json2bufrd']['user']
groupamqp2amqp_json2bufrd                = config['amqp2amqp_json2bufrd']['group']

# section amqp2djangod
logfileamqp2djangod              = config['amqp2djangod']['logfile']
errfileamqp2djangod              = config['amqp2djangod']['errfile']
lockfileamqp2djangod             = config['amqp2djangod']['lockfile']
useramqp2djangod                 = config['amqp2djangod']['user']
groupamqp2djangod                = config['amqp2djangod']['group']

# section amqp2geoimaged
logfileamqp2geoimaged              = config['amqp2geoimaged']['logfile']
errfileamqp2geoimaged              = config['amqp2geoimaged']['errfile']
lockfileamqp2geoimaged             = config['amqp2geoimaged']['lockfile']
useramqp2geoimaged                 = config['amqp2geoimaged']['user']
groupamqp2geoimaged                = config['amqp2geoimaged']['group']

# section amqp2arkimetd
logfileamqp2arkimetd              = config['amqp2arkimetd']['logfile']
errfileamqp2arkimetd              = config['amqp2arkimetd']['errfile']
lockfileamqp2arkimetd             = config['amqp2arkimetd']['lockfile']
useramqp2arkimetd                 = config['amqp2arkimetd']['user']
groupamqp2arkimetd                = config['amqp2arkimetd']['group']

# section amqp2mqttd
logfileamqp2mqttd              = config['amqp2mqttd']['logfile']
errfileamqp2mqttd              = config['amqp2mqttd']['errfile']
lockfileamqp2mqttd             = config['amqp2mqttd']['lockfile']
useramqp2mqttd                 = config['amqp2mqttd']['user']
groupamqp2mqttd                = config['amqp2mqttd']['group']

# section mqtt2dballed
logfilemqtt2dballed              = config['mqtt2dballed']['logfile']
errfilemqtt2dballed              = config['mqtt2dballed']['errfile']
lockfilemqtt2dballed             = config['mqtt2dballed']['lockfile']
usermqtt2dballed                 = config['mqtt2dballed']['user']
groupmqtt2dballed                = config['mqtt2dballed']['group']
dsnsample_fixed                  = config['mqtt2dballed']['dsnsample_fixed']
dsnsample_mobile                 = config['mqtt2dballed']['dsnsample_mobile']
dsnreport_fixed                  = config['mqtt2dballed']['dsnreport_fixed']
dsnreport_mobile                 = config['mqtt2dballed']['dsnreport_mobile']
topicsample                        = config['mqtt2dballed']['topicsample']
topicreport                        = config['mqtt2dballed']['topicreport']

# section composereportd
logfilecomposereportd              = config['composereportd']['logfile']
errfilecomposereportd              = config['composereportd']['errfile']
lockfilecomposereportd             = config['composereportd']['lockfile']
usercomposereportd                 = config['composereportd']['user']
groupcomposereportd                = config['composereportd']['group']
exchangecomposereportd             = config['composereportd']['exchange']


# section database
DATABASE_USER     = config['database']['DATABASE_USER']
DATABASE_PASSWORD = config['database']['DATABASE_PASSWORD']
DATABASE_HOST     = config['database']['DATABASE_HOST']
DATABASE_PORT     = config['database']['DATABASE_PORT']
DATABASE_ENGINE   = config['database']['DATABASE_ENGINE']
DATABASE_NAME     = config['database']['DATABASE_NAME']

# section stationd
logfilestationd              = config['stationd']['logfile']
errfilestationd              = config['stationd']['errfile']
lockfilestationd             = config['stationd']['lockfile']
conffilestationd             = config['stationd']['conffile']
userstationd                 = config['stationd']['user']
groupstationd                = config['stationd']['group']
stationslug                  = config['stationd']['stationslug']
ident                        = config['stationd']['ident']
boardslug                    = config['stationd']['boardslug']

# section mqtt2graphited
logfilemqtt2graphited              = config['mqtt2graphited']['logfile']
errfilemqtt2graphited              = config['mqtt2graphited']['errfile']
lockfilemqtt2graphited             = config['mqtt2graphited']['lockfile']
usermqtt2graphited                 = config['mqtt2graphited']['user']
groupmqtt2graphited                = config['mqtt2graphited']['group']
mapfilemqtt2graphited              = config['mqtt2graphited']['mapfile']



#######               graphite settings

from os.path import abspath, dirname, join

WEBAPP_VERSION="rmap "+__version__ + " + graphite git 01/01/2017"
JAVASCRIPT_DEBUG = False
DATE_FORMAT = '%m/%d'
WEB_DIR = dirname( abspath(__file__) )
WEBAPP_DIR = dirname(WEB_DIR)
GRAPHITE_ROOT = dirname(WEBAPP_DIR)
DEFAULT_CACHE_DURATION = 60 #metric data and graphs are cached for one minute by default
LOG_CACHE_PERFORMANCE = False
LOG_ROTATION = True
LOG_ROTATION_COUNT = 1
MAX_FETCH_RETRIES = 2
DOCUMENTATION_URL = "http://graphite.readthedocs.io/"
ALLOW_ANONYMOUS_CLI = True
LEGEND_MAX_ITEMS = 10
RRD_CF = 'AVERAGE'
#STORAGE_FINDERS = (
#    'graphite-dballe.finders.standard.StandardFinder',
#)
STORAGE_FINDERS = (
    'graphite-dballe.finders.dballe.DballeFinderSampleFixed',
    'graphite-dballe.finders.dballe.DballeFinderSampleMobile',
    'graphite-dballe.finders.dballe.DballeFinderReportFixed',
    'graphite-dballe.finders.dballe.DballeFinderReportMobile',
)

MAX_TAG_LENGTH = 50
AUTO_REFRESH_INTERVAL = 60
# Set to True to require authentication to save or delete dashboards
DASHBOARD_REQUIRE_AUTHENTICATION = False
# Require Django change/delete permissions to save or delete dashboards.
# NOTE: Requires DASHBOARD_REQUIRE_AUTHENTICATION to be set
DASHBOARD_REQUIRE_PERMISSIONS = False
# Name of a group to which the user must belong to save or delete dashboards.  Alternative to
# DASHBOARD_REQUIRE_PERMISSIONS, particularly useful when using only LDAP (without Admin app)
# NOTE: Requires DASHBOARD_REQUIRE_AUTHENTICATION to be set
DASHBOARD_REQUIRE_EDIT_GROUP = None

CONF_DIR = ''
DASHBOARD_CONF = ''
GRAPHTEMPLATES_CONF = ''
STORAGE_DIR = ''
WHITELIST_FILE = ''
INDEX_FILE = ''
LOG_RENDERING_PERFORMANCE = False
CARBONLINK_HOSTS = ["127.0.0.1:7002"]
CARBONLINK_TIMEOUT = 1.0
CARBONLINK_HASHING_KEYFUNC = None
CARBONLINK_HASHING_TYPE = 'carbon_ch'
CARBONLINK_RETRY_DELAY = 15
REPLICATION_FACTOR = 1
STANDARD_DIRS = []
# Cluster settings
CLUSTER_SERVERS = []
URL_PREFIX = ''
DEFAULT_CACHE_POLICY = []
#Remote rendering settings
REMOTE_RENDERING = False #if True, rendering is delegated to RENDERING_HOSTS
RENDERING_HOSTS = []
REMOTE_RENDER_CONNECT_TIMEOUT = 1.0
LOG_RENDERING_PERFORMANCE = False
FIND_CACHE_DURATION = 300
FIND_TOLERANCE = 2 * FIND_CACHE_DURATION
REMOTE_STORE_MERGE_RESULTS = True

if not CONF_DIR:
  CONF_DIR = os.environ.get('GRAPHITE_CONF_DIR', join(GRAPHITE_ROOT, 'conf'))
if not DASHBOARD_CONF:
  DASHBOARD_CONF = join(CONF_DIR, 'dashboard.conf')
if not GRAPHTEMPLATES_CONF:
  GRAPHTEMPLATES_CONF = join(CONF_DIR, 'graphTemplates.conf')
if not STORAGE_DIR:
  STORAGE_DIR = os.environ.get('GRAPHITE_STORAGE_DIR', join(GRAPHITE_ROOT, 'storage'))
if not WHITELIST_FILE:
  WHITELIST_FILE = join(STORAGE_DIR, 'lists', 'whitelist')
if not INDEX_FILE:
  INDEX_FILE = join(STORAGE_DIR, 'index')

###  end graphite settings


if DATABASE_ENGINE == "mysql":
    # Recommended for MySQL. See http://code.djangoproject.com/ticket/13906
    # to avoid "Lost connection to MySQL server at 'reading authorization packet', system error: 0"
    # connect_timeout=30
    DATABASES = {
        'default': {
            'ENGINE': 'django.db.backends.'+DATABASE_ENGINE,
            'NAME':    DATABASE_NAME,
            'USER':    DATABASE_USER,
            'PASSWORD':DATABASE_PASSWORD,
            'HOST':    DATABASE_HOST,
            'PORT':    DATABASE_PORT,
            'OPTIONS': {'init_command': 'SET SESSION TRANSACTION ISOLATION LEVEL READ COMMITTED',
                        'connect_timeout':60},
            }
        }
else:
    DATABASES = {
        'default': {
            'ENGINE': 'django.db.backends.'+DATABASE_ENGINE,
            'NAME':    DATABASE_NAME,
            'USER':    DATABASE_USER,
            'PASSWORD':DATABASE_PASSWORD,
            'HOST':    DATABASE_HOST,
            'PORT':    DATABASE_PORT,
            }
        }



# Additional locations of static files
STATICFILES_DIRS = [
    "global_static",
]


# List of finder classes that know how to find static files in
# various locations.
STATICFILES_FINDERS = [
    'django.contrib.staticfiles.finders.FileSystemFinder',
    'django.contrib.staticfiles.finders.AppDirectoriesFinder',
#    'django.contrib.staticfiles.finders.DefaultStorageFinder',
]


MIDDLEWARE_CLASSES = [
    'django_hosts.middleware.HostsRequestMiddleware',
    'django.middleware.cache.UpdateCacheMiddleware',
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.locale.LocaleMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
#    'django.middleware.doc.XViewMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware',
    'django.middleware.cache.FetchFromCacheMiddleware',
    'django_hosts.middleware.HostsResponseMiddleware',
]

TEMPLATES= [
    {
        'BACKEND': 'django.template.backends.django.DjangoTemplates',
        'DIRS'  : [],
        'APP_DIRS' : True,
        'OPTIONS': {
            # List of callables that know how to import templates from various sources.
            'context_processors': [
                'rmap.processor.site',
                'django.contrib.auth.context_processors.auth',
                'django.template.context_processors.debug',
                'django.template.context_processors.i18n',
                'django.template.context_processors.media',
#                'django.core.context_processors.request',
                'django.template.context_processors.static',
                'django.template.context_processors.tz',
                'django.contrib.messages.context_processors.messages',
            ],
            'debug' : config['django']['TEMPLATE_DEBUG']
        }
    }
]

ROOT_URLCONF = 'rmap.urls'

INSTALLED_APPS = [
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.sites',
    'django.contrib.messages',
    'django.contrib.admin',
#    'django.contrib.admindocs',
    'django.contrib.staticfiles',
    'rmap.doc',
    'rmap',
    'rmap.stations',
    'registration',
    'django_hosts'
]

# if not android :
#     INSTALLED_APPS += [
#         'leaflet',
#         'djgeojson',
#         'geoimage',
#         'insertdata',
#         'imagekit',
#         'showdata',
#         'amatyr',
#         'borinud',
#         'cookielaw',
#     ]



# django save the files on memory, but large files are saved in a path.
# The size of "large file" can be defined in settings using
# FILE_UPLOAD_MAX_MEMORY_SIZE and The FILE_UPLOAD_HANDLERS by default are:
#("django.core.files.uploadhandler.MemoryFileUploadHandler",
# "django.core.files.uploadhandler.TemporaryFileUploadHandler",)

# remove MemoryFileUploadHandler
FILE_UPLOAD_HANDLERS = [
"django.core.files.uploadhandler.TemporaryFileUploadHandler",]

#for django < 1.6 only
AUTH_PROFILE_MODULE = 'stations.UserProfile'

LOGIN_URL = '/registrazione/login'

#from django.conf import global_settings

#to avoid this message:
#CommandError: Unable to serialize database: <User: rmap> is not JSON serializable
#SESSION_SERIALIZER = 'django.contrib.sessions.serializers.PickleSerializer'

SERIALIZATION_MODULES = {
    'geojson' : 'djgeojson.serializers'
}

#CACHES = {
#    'default': {
#        'BACKEND': 'django.core.cache.backerlnds.memcached.MemcachedCache',
#        'LOCATION': '127.0.0.1:11211',
#    }
#}

if not android :
    CACHES = {
        'default': {
            'BACKEND': 'django.core.cache.backends.filebased.FileBasedCache',
            'LOCATION': '/var/tmp/django_cache',
        }
    }


sample_measurements=[
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B10004",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B11001",
        "level": (103, 10000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B11002",
        "level": (103, 10000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13011",
        "level": (1, None, None, None),
        "trange": (1, 0, 3600),
    },
    {
        "var": "B15198",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B15195",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B20003",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13013",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B20001",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 60),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 60),
    },

]

report_measurements=[
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 900),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 900),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 900),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 900),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 900),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 900),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 1800),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 1800),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 1800),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 1800),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 1800),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 1800),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 3600),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 3600),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 3600),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (2, 0, 3600),
    },
    {
        "var": "B12101",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 3600),
    },
    {
        "var": "B13003",
        "level": (103, 2000, None, None),
        "trange": (3, 0, 3600),
    },
    {
        "var": "B10004",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B11001",
        "level": (103, 10000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B11002",
        "level": (103, 10000, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13011",
        "level": (1, None, None, None),
        "trange": (1, 0, 3600),
    },
    {
        "var": "B15198",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 900),
    },
    {
        "var": "B15195",
        "level": (103, 2000, None, None),
        "trange": (0, 0, 900),
    },
    {
        "var": "B20003",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B13013",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
    {
        "var": "B20001",
        "level": (1, None, None, None),
        "trange": (254, 0, 0),
    },
]


BORINUD =\
          {"report":{
              "SOURCES": 
              [
                  {
                      "class": "borinud.utils.source.DballeDB",
                      "url": dsnreport_fixed,
                  }, 
                  {
                      "class": "borinud.utils.source.DballeDB",
                      "url": dsnreport_mobile,
                  }, 
                  {
                      "class": "borinud.utils.source.ArkimetBufrDB",
                      "dataset": "http://rmap.cc:8090/dataset/meteonetwork",
                      "measurements": report_measurements
                  },
                  {
                      "class": "borinud.utils.source.ArkimetBufrDB",
                      "dataset": "http://rmap.cc:8090/dataset/arpav",
                      "measurements": report_measurements
                  },
                  {
                      "class": "borinud.utils.source.ArkimetBufrDB",
                      "dataset": "http://rmap.cc:8090/dataset/opendata-er",
                      "measurements": report_measurements
                  },        
                  {
                      "class": "borinud.utils.source.ArkimetBufrDB",
                      "dataset": "http://rmap.cc:8090/dataset/report_fixed",
                      "measurements": report_measurements
                  },
                  {
                      "class": "borinud.utils.source.ArkimetBufrDB",
                      "dataset": "http://rmap.cc:8090/dataset/report_mobile",
                      "measurements": report_measurements
                  },
              ],
              "CACHED_SUMMARY": "default",
              "CACHED_SUMMARY_TIMEOUT": 60*15,},
           
           "report_fixed":{
               "SOURCES": 
               [
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnreport_fixed,
                   }, 
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/meteonetwork",
                       "measurements": report_measurements
                   },
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/arpav",
                       "measurements": report_measurements
                   },
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/opendata-er",
                       "measurements": report_measurements
                   },        
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/report_fixed",
                       "measurements": report_measurements
                   },
               ],
               "CACHED_SUMMARY": "default",
               "CACHED_SUMMARY_TIMEOUT": 60*15,
           },
           "report_mobile":{
               "SOURCES": 
               [
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnreport_mobile,
                   }, 
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/report_mobile",
                       "measurements": report_measurements
                   },
               ],
               "CACHED_SUMMARY": "default",
               "CACHED_SUMMARY_TIMEOUT": 60*15,
           },
           "sample":{
               "SOURCES": 
               [
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnsample_fixed,
                   }, 
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnsample_mobile,
                   },         
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/sample_fixed",
                       "measurements": sample_measurements
                   },
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/sample_mobile",
                       "measurements": sample_measurements
                   },
               ],
               "CACHED_SUMMARY": "default",
               "CACHED_SUMMARY_TIMEOUT": 60*15,
           },
           "sample_fixed":{
               "SOURCES": 
               [
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnsample_fixed,
                   }, 
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/sample_fixed",
                       "measurements": sample_measurements
                   },
               ],
               "CACHED_SUMMARY": "default",
               "CACHED_SUMMARY_TIMEOUT": 60*15,
           },
           "sample_mobile":{
               "SOURCES": 
               [
                   {
                       "class": "borinud.utils.source.DballeDB",
                       "url": dsnsample_mobile,
                   },         
                   {
                       "class": "borinud.utils.source.ArkimetBufrDB",
                       "dataset": "http://rmap.cc:8090/dataset/sample_mobile",
                       "measurements": sample_measurements
                   },
               ],
               "CACHED_SUMMARY": "default",
               "CACHED_SUMMARY_TIMEOUT": 60*15,
           }
          }


if DEBUG:
    BORINUD =\
              {
                  "report":
                  {
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnreport_fixed,
                          }, 
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnreport_mobile,
                          }, 
                          
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,},
                  
                  "report_fixed":{
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnreport_fixed,
                          }, 
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,
                  },
                  "report_mobile":{
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnreport_mobile,
                          }, 
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,
                  },
                  "sample":{
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnsample_fixed,
                          }, 
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnsample_mobile,
                          }, 
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,
                  },
                  "sample_fixed":{
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnsample_fixed,
                          }, 
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,
                  },
                  "sample_mobile":{
                      "SOURCES": 
                      [
                          {
                              "class": "borinud.utils.source.DballeDB",
                              "url": dsnsample_mobile,
                          },         
                      ],
                      "CACHED_SUMMARY": "default",
                      "CACHED_SUMMARY_TIMEOUT": 60*15,
                  }
              }
    
    
SHOWDATA = BORINUD


LEAFLET_CONFIG = {
#    'SPATIAL_EXTENT': (0.0, 30.0, 30, 60),
'DEFAULT_CENTER': (41.5, 11.0),
'MIN_ZOOM': 2,
'DEFAULT_ZOOM': 5,
'MAX_ZOOM': 19,
#'RESET_VIEW': False,
#'MINIMAP': True,
}

MAINSITES=("rmapv.rmap.cc","rmap.publicwifi.it")
ALLOWED_HOSTS = ['*']

#django-hosts configuration
ROOT_HOSTCONF = 'rmap.hosts'
DEFAULT_HOST = 'rmapv'  # Name of the default host


LOAD_OPTIONAL_APPS = not android

if LOAD_OPTIONAL_APPS:
    # <copypaste from="https://gist.github.com/msabramo/945406">
    # Define any settings specific to each of the optional apps.

    # Sequence for each optional app as a dict containing info about the app.
    OPTIONAL_APPS = (

        #{"import": module, "apps":(app,), "condition": bool, "middleware":(middle,), "context_processors": (processor,) }

        {"import": 'leaflet',   "apps": ('leaflet',)},
        {"import": 'djgeojson', "apps": ('djgeojson' ,)},
        {"import": 'geoimage',  "apps": ('geoimage'  ,)},
        {"import": 'insertdata',"apps": ('insertdata',)},
        {"import": 'imagekit',  "apps": ('imagekit'  ,)},
        {"import": 'showdata',  "apps": ('showdata'  ,)},
        {"import": 'amatyr',    "apps": ('amatyr'    ,)},
        {"import": 'borinud',   "apps": ('borinud'   ,)},
        {"import": 'http2mqtt', "apps": ('http2mqtt' ,)},
#        {"import": 'cookielaw', "apps": ('cookielaw' ,) ,"context_processors": ('django.core.context_processors.request', )},
        {"import": 'cookielaw', "apps": ('cookielaw' ,) ,"context_processors": ('django.template.context_processors.request', )},
        {"import": 'graphite-dballe',                "apps": ('graphite-dballe',)},
        {"import": 'graphite-dballe.metrics',        "apps": ('graphite-dballe.metrics',)},
        {"import": 'graphite-dballe.render',         "apps": ('graphite-dballe.render',)},
        {"import": 'graphite-dballe.browser',        "apps": ('graphite-dballe.browser',)},
        {"import": 'graphite-dballe.composer',       "apps": ('graphite-dballe.composer',)},
        {"import": 'graphite-dballe.account',        "apps": ('graphite-dballe.account',)},
        {"import": 'graphite-dballe.dashboard',      "apps": ('graphite-dballe.dashboard',)},
        {"import": 'graphite-dballe.whitelist',      "apps": ('graphite-dballe.whitelist',)},
        {"import": 'graphite-dballe.events',         "apps": ('graphite-dballe.events',)},
        {"import": 'graphite-dballe.url_shortener',  "apps": ('graphite-dballe.url_shortener',)},
        {"import": 'tagging',                        "apps": ('tagging',)},
        {"import": 'django_extensions',              "apps": ('django_extensions',)},
        {"import": 'rainbo',                         "apps": ('rainbo'   ,)},
        {"import": 'borinud_sos',                    "apps": ('borinud_sos'   ,)},
    )

    # Set up each optional app if available.
    for app in OPTIONAL_APPS:
        if app.get("condition", True):
            try:
                moduletree= app["import"].split(".")
                module_info=imp.find_module(moduletree[0])
                if len(moduletree) >1:
                    module = imp.load_module(moduletree[0], *module_info)
                    imp.find_module(moduletree[1], module.__path__) # __path__ is already a list

            except ImportError:
                print "import error: ", app["import"]
                print "disable     : ", app.get("apps", ())
            else:
                print "enable      : ", app.get("apps", ())
                INSTALLED_APPS += app.get("apps", ())
                MIDDLEWARE_CLASSES += app.get("middleware", ())
                TEMPLATES[0]['OPTIONS']['context_processors']+= app.get("context_processors", ())


LOGGING = {
    'version': 1,
    'disable_existing_loggers': False,
    'handlers': {
        'rot_file':{
            'level': 'DEBUG',
            'class': 'logging.handlers.RotatingFileHandler',
            'filename': '/tmp/django_rot.log',
            'maxBytes': '16777216', # 16megabytes
            'formatter': 'verbose'
        },
    },
    'formatters': {
        'verbose': {
            'style': '{',
            'format': '%(levelname)s %(asctime)s %(name)s.%(funcName)s:%(lineno)s- %(message)s'
        },
    },
    'loggers': {
        'django': {
            'handlers': ['rot_file'],
            'level': 'ERROR',
            'propagate': True,
        },
    },
}

