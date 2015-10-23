from django.conf.urls import *
import settings
from stations.form import RmapRegistrationForm
#from registration.forms import RegistrationFormTermsOfService
from registration.backends.default.views import RegistrationView
from views import home,wizard,wizard2,wizard_done,wizard_error

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',

    url(r'^$',home ,name='home' ),
    url(r'^wizard/$',wizard ,name='wizard' ),
    url(r'^wizard2/$',wizard2 ,name='wizard2' ),
    url(r'^wizard_done/$',wizard_done ,name='wizard_done' ),
    url(r'^wizard_error/$',wizard_error ,name='wizard_error' ),
#    Uncomment the next line to enable admin documentation:
    url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

#    Uncomment the next line to enable the admin:
    url(r'^admin/', include(admin.site.urls)),

    url(r'^', include('http2mqtt.urls')),

    url(r'^', include('rmap.stations.urls')),

#    override default register form
#    url(r'^registrazione/register/$', RegistrationView.as_view(form_class=RegistrationFormTermsOfService), name='registration_register'),
    url(r'^registrazione/register/$', RegistrationView.as_view(form_class= RmapRegistrationForm),name='registration_register'),

    url(r'^registrazione/', include('registration.backends.default.urls')),
    url(r'^auth/user',     'rmap.views.user'),
    url(r'^auth/vhost',    'rmap.views.vhost'),
    url(r'^auth/resource', 'rmap.views.resource'),

    url(r'^auth/auth',     'rmap.views.auth'),
    url(r'^auth/superuser','rmap.views.superuser'),
    url(r'^auth/acl',      'rmap.views.acl'),


    url(r'^accounts/profile/$',      'rmap.views.profile'),
    url(r'^accounts/profile/(?P<mystation_slug>[-\w]+)/$',      'rmap.views.profile_details'),

    url(r'^http2mqtt/', include('http2mqtt.urls')),
)


if ( settings.SERVE_STATIC ):
#serve local static files
    urlpatterns += patterns('',
                            (r'^'+settings.MEDIA_PREFIX[1:]+'(.*)', 'django.views.static.serve', {'document_root': settings.MEDIA_ROOT, 'show_indexes': True}),
                            (r'^'+settings.MEDIA_SITE_PREFIX[1:]+'(.*)', 'django.views.static.serve', {'document_root': settings.MEDIA_SITE_ROOT, 'show_indexes': True}),
                            )

    #To use the view with a different local development server, 
    #add the following helper function that'll do this for you to the end of 
    #your primary URL configuration

    from django.contrib.staticfiles.urls import staticfiles_urlpatterns
    urlpatterns += staticfiles_urlpatterns()
