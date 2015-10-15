# Django settings for rmap project.

import os
from configobj import ConfigObj,flatten_errors
from validate import Validator

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


configspec['django']['TIME_ZONE']="string(default='Europe/Rome')"
configspec['django']['LANGUAGE_CODE']="string(default='en-us')"
configspec['django']['SITE_ID']="integer(default=1)"
configspec['django']['USE_I18N']="boolean(default=True)"
configspec['django']['LOCALE_PATHS']="list(default=list('locale',))"
configspec['django']['ADMINS']="list(default=list('',))"
configspec['django']['MANAGERS']="list(default=list('',))"
configspec['django']['MEDIA_ROOT']="string(default='%s/media/')" % os.getcwd()
configspec['django']['MEDIA_SITE_ROOT']="string(default='%s/media/')" % os.getcwd()
configspec['django']['TEMPLATE_DIRS']="list(default=list('templates',))"
configspec['django']['BASE_URL']="string(default='/django/')"
configspec['django']['ADMIN_MEDIA_PREFIX']="string(default='/django/media/admin/')"
configspec['django']['STATIC_URL']="string(default='/static/')"
configspec['django']['STATIC_ROOT'] = "string(default='/usr/lib/python2.7/site-packages/django/contrib/admin/static/admin/')"
configspec['django']['MEDIA_PREFIX']="string(default='/static/')"
configspec['django']['MEDIA_SITE_PREFIX']="string(default='/static/sito/')"
configspec['django']['SERVE_STATIC']="boolean(default=True)"


configspec['daemon']={}

configspec['daemon']['amqpuser']        = "string(default='rmap')"
configspec['daemon']['amqppassword']    = "string(default='rmap')"


configspec['rmapweb']={}

configspec['rmapweb']['logfile']  = "string(default='/tmp/rmapweb.log')"
configspec['rmapweb']['errfile']  = "string(default='/tmp/rmapweb.err')"
configspec['rmapweb']['lockfile'] = "string(default='/tmp/rmapweb.lock')"
configspec['rmapweb']['user']     = "string(default=None)"
configspec['rmapweb']['group']    = "string(default=None)"
configspec['rmapweb']['port']    = "string(default='8080')"


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
configspec['stationd']['boardslug']    = "string(default='base')"


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

configspec['amqp2amqp_identvalidationd']={}
configspec['amqp2amqp_identvalidationd']['logfile']  = "string(default='/tmp/amqp2amqp_identvalidationd.log')"
configspec['amqp2amqp_identvalidationd']['errfile']  = "string(default='/tmp/amqp2amqp_identvalidationd.err')"
configspec['amqp2amqp_identvalidationd']['lockfile'] = "string(default='/tmp/amqp2amqp_identvalidationd.lock')"
configspec['amqp2amqp_identvalidationd']['user']     = "string(default=None)"
configspec['amqp2amqp_identvalidationd']['group']    = "string(default=None)"

configspec['amqp2djangod']={}
configspec['amqp2djangod']['logfile']  = "string(default='/tmp/amqp2django.log')"
configspec['amqp2djangod']['errfile']  = "string(default='/tmp/amqp2django.err')"
configspec['amqp2djangod']['lockfile'] = "string(default='/tmp/amqp2django.lock')"
configspec['amqp2djangod']['user']     = "string(default=None)"
configspec['amqp2djangod']['group']    = "string(default=None)"



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

configspec['borinudd']={}
configspec['borinudd']['logfile']  = "string(default='/tmp/borinudd.log')"
configspec['borinudd']['errfile']  = "string(default='/tmp/borinudd.err')"
configspec['borinudd']['lockfile'] = "string(default='/tmp/borinudd.lock')"
configspec['borinudd']['user']     = "string(default=None)"
configspec['borinudd']['group']    = "string(default=None)"

configspec['mqtt2dballed']={}
configspec['mqtt2dballed']['logfile']  = "string(default='/tmp/mqtt2dballed.log')"
configspec['mqtt2dballed']['errfile']  = "string(default='/tmp/mqtt2dballed.err')"
configspec['mqtt2dballed']['lockfile'] = "string(default='/tmp/mqtt2dballed.lock')"
configspec['mqtt2dballed']['user']     = "string(default=None)"
configspec['mqtt2dballed']['group']    = "string(default=None)"

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
TEMPLATE_DEBUG          = config['django']['TEMPLATE_DEBUG']
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
TEMPLATE_DIRS           = config['django']['TEMPLATE_DIRS']
BASE_URL                = config['django']['BASE_URL']
ADMIN_MEDIA_PREFIX      = config['django']['ADMIN_MEDIA_PREFIX']
STATIC_URL              = config['django']['STATIC_URL']
STATIC_ROOT             = config['django']['STATIC_ROOT']
MEDIA_PREFIX            = config['django']['MEDIA_PREFIX']
MEDIA_SITE_PREFIX       = config['django']['MEDIA_SITE_PREFIX']
SERVE_STATIC            = config['django']['SERVE_STATIC']
MEDIA_URL               = BASE_URL+MEDIA_PREFIX
SITE_MEDIA_URL          = BASE_URL+MEDIA_SITE_PREFIX


# section daemon
amqpuser                = config['daemon']['amqpuser']
amqppassword            = config['daemon']['amqppassword']

# section rmapweb
logfileweb              = config['rmapweb']['logfile']
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

# section amqp2amqp_identvalidationd
logfileamqp2amqp_identvalidationd              = config['amqp2amqp_identvalidationd']['logfile']
errfileamqp2amqp_identvalidationd              = config['amqp2amqp_identvalidationd']['errfile']
lockfileamqp2amqp_identvalidationd             = config['amqp2amqp_identvalidationd']['lockfile']
useramqp2amqp_identvalidationd                 = config['amqp2amqp_identvalidationd']['user']
groupamqp2amqp_identvalidationd                = config['amqp2amqp_identvalidationd']['group']


# section amqp2djangod
logfileamqp2djangod              = config['amqp2djangod']['logfile']
errfileamqp2djangod              = config['amqp2djangod']['errfile']
lockfileamqp2djangod             = config['amqp2djangod']['lockfile']
useramqp2djangod                 = config['amqp2djangod']['user']
groupamqp2djangod                = config['amqp2djangod']['group']

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

# section borinudd
logfileborinudd              = config['borinudd']['logfile']
errfileborinudd              = config['borinudd']['errfile']
lockfileborinudd             = config['borinudd']['lockfile']
userborinudd                 = config['borinudd']['user']
groupborinudd                = config['borinudd']['group']

# section mqtt2dballed
logfilemqtt2dballed              = config['mqtt2dballed']['logfile']
errfilemqtt2dballed              = config['mqtt2dballed']['errfile']
lockfilemqtt2dballed             = config['mqtt2dballed']['lockfile']
usermqtt2dballed                 = config['mqtt2dballed']['user']
groupmqtt2dballed                = config['mqtt2dballed']['group']

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
boardslug                    = config['stationd']['boardslug']

# section mqtt2graphited
logfilemqtt2graphited              = config['mqtt2graphited']['logfile']
errfilemqtt2graphited              = config['mqtt2graphited']['errfile']
lockfilemqtt2graphited             = config['mqtt2graphited']['lockfile']
usermqtt2graphited                 = config['mqtt2graphited']['user']
groupmqtt2graphited                = config['mqtt2graphited']['group']
mapfilemqtt2graphited              = config['mqtt2graphited']['mapfile']


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
STATICFILES_DIRS = (
    # Put strings here, like "/home/html/static" or "C:/www/django/static".
    # Always use forward slashes, even on Windows.
    # Don't forget to use absolute paths, not relative paths.
    'static/',
)


# List of finder classes that know how to find static files in
# various locations.
STATICFILES_FINDERS = (
    'django.contrib.staticfiles.finders.FileSystemFinder',
    'django.contrib.staticfiles.finders.AppDirectoriesFinder',
#    'django.contrib.staticfiles.finders.DefaultStorageFinder',
)




# List of callables that know how to import templates from various sources.
TEMPLATE_LOADERS = (
    'django.template.loaders.filesystem.Loader',
    'django.template.loaders.app_directories.Loader',
#     'django.template.loaders.eggs.load_template_source',
)

MIDDLEWARE_CLASSES = (
    'django.contrib.sessions.middleware.SessionMiddleware',
    'django.middleware.locale.LocaleMiddleware',
    'django.middleware.common.CommonMiddleware',
    'django.middleware.csrf.CsrfViewMiddleware',
    'django.contrib.auth.middleware.AuthenticationMiddleware',
#    'django.middleware.doc.XViewMiddleware',
    'django.contrib.messages.middleware.MessageMiddleware')

ROOT_URLCONF = 'rmap.urls'

INSTALLED_APPS = (
    'django.contrib.auth',
    'django.contrib.contenttypes',
    'django.contrib.sessions',
    'django.contrib.sites',
    'django.contrib.messages',
    'registration',
    'django.contrib.admin',
#    'django.contrib.admindocs',
    'django.contrib.staticfiles',
    'rmap.stations',
    'rmap.doc',
)

# django save the files on memory, but large files are saved in a path.
# The size of "large file" can be defined in settings using
# FILE_UPLOAD_MAX_MEMORY_SIZE and The FILE_UPLOAD_HANDLERS by default are:
#("django.core.files.uploadhandler.MemoryFileUploadHandler",
# "django.core.files.uploadhandler.TemporaryFileUploadHandler",)

# remove MemoryFileUploadHandler
FILE_UPLOAD_HANDLERS = (
"django.core.files.uploadhandler.TemporaryFileUploadHandler",)

try:
    import django_extensions
    INSTALLED_APPS += 'django_extensions',
except ImportError:
    print "django_extensions is not installed; I do not use it"
    pass

#for django < 1.6 only
AUTH_PROFILE_MODULE = 'stations.UserProfile'

LOGIN_URL = '/registrazione/login'

from django.conf import global_settings
TEMPLATE_CONTEXT_PROCESSORS = global_settings.TEMPLATE_CONTEXT_PROCESSORS + (
    "rmap.processor.site",
)

#to avoid this message:
#CommandError: Unable to serialize database: <User: rmap> is not JSON serializable
#SESSION_SERIALIZER = 'django.contrib.sessions.serializers.PickleSerializer'
