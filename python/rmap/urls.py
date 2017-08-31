from django.conf.urls import *
import settings
from stations.form import RmapRegistrationForm
#from registration.forms import RegistrationFormTermsOfService
from registration.backends.default.views import RegistrationView
from views import home,wizard,wizard2,wizard_done,wizard_error
import rmap.views
import django.views
from django.conf.urls.static import static
from django.views.generic import TemplateView

# Uncomment the next two lines to enable the admin:
from django.contrib import admin
admin.autodiscover()

from os import environ
android=('ANDROID_ARGUMENT' in environ)

urlpatterns = [
    url(r'^$',home ,name='home' ),
    url(r'^wizard/$',wizard ,name='wizard' ),
    url(r'^wizard2/$',wizard2 ,name='wizard2' ),
    url(r'^wizard_done/$',wizard_done ,name='wizard_done' ),
    url(r'^wizard_error/$',wizard_error ,name='wizard_error' ),
#    Uncomment the next line to enable admin documentation:
    url(r'^admin/doc/', include('django.contrib.admindocs.urls')),

#    Uncomment the next line to enable the admin:
    url(r'^admin/', include(admin.site.urls)),

    url(r'^', include('rmap.stations.urls')),

#    override default register form
#    url(r'^registrazione/register/$', RegistrationView.as_view(form_class=RegistrationFormTermsOfService), name='registration_register'),
    url(r'^registrazione/register/$', RegistrationView.as_view(form_class= RmapRegistrationForm),name='registration_register'),

    url(r'^registrazione/', include('registration.backends.default.urls')),
    url(r'^auth/user',     rmap.views.user),
    url(r'^auth/vhost',    rmap.views.vhost),
    url(r'^auth/resource', rmap.views.resource),

    url(r'^auth/auth',     rmap.views.auth),
    url(r'^auth/superuser',rmap.views.superuser),
    url(r'^auth/acl',      rmap.views.acl),


    url(r'^accounts/profile/$',      rmap.views.profile),
    url(r'^accounts/profile/(?P<mystation_slug>[-\w]+)/$',      rmap.views.profile_details),
    url(r'^robots.txt$', TemplateView.as_view(template_name="robots.txt", content_type="text/plain"), name="robots_file")
]

if not android  :
    #try:
    #    urlpatterns.append(url(r'^rainbo/', include('rainbo.urls')))
    #    #del urlpatterns[0]
    #except Exception as e:
    #    print "Warnig: rainbo disabled"
    #    print e        
    try:
        urlpatterns.append(url(r'^', include('http2mqtt.urls')))
    except Exception as e:
        print "Warnig: http2mqtt disabled"
        print e
    try:
        urlpatterns.append(url(r'^borinud/', include('borinud.urls')))
    except Exception as e:
        print "Warnig: borinud disabled"
        print e
    try:
        urlpatterns.append(url(r'^geoimage/', include('geoimage.urls')))
    except Exception as e:
        print "Warnig: geoimage disabled"
        print e
    try:
        urlpatterns.append(url(r'^insertdata/', include('insertdata.urls')))
    except Exception as e:
        print "Warnig: insertdata disabled"
        print e
    try:
        urlpatterns.append(url(r'^amatyr/', include('amatyr.urls')))
    except Exception as e:
        print "Warnig: amatyr disabled"
        print e

    try:
        urlpatterns.append(url(r'^showdata/', include('showdata.urls',namespace="showdata")))
    except Exception as e:
        print "Warnig: showdata disabled"
        print e

    try:
        #urlpatterns.append(url(r'^graphite/', include('graphite-dballe.urls',namespace="graphite")))
        urlpatterns.append(url(r'^graphite/', include('graphite-dballe.urls')))
    except Exception as e:
        print "Warnig: graphite disabled"
        print e

    try:
        #urlpatterns.append(url(r'^graphite/', include('graphite-dballe.urls',namespace="graphite")))
        urlpatterns.append(url(r'^sos/', include('borinud_sos.urls')))
    except Exception as e:
        print "Warnig: sos disabled"
        print e
        
  
        
if ( settings.SERVE_STATIC ):
#serve local static files
    from django.contrib.staticfiles import views
    urlpatterns += static(settings.MEDIA_URL, document_root=settings.MEDIA_ROOT)
    urlpatterns += [
                            url(r'^'+settings.MEDIA_PREFIX[1:]+'(.*)', views.serve, {'document_root': settings.MEDIA_ROOT, 'show_indexes': True}),
                            url(r'^'+settings.MEDIA_SITE_PREFIX[1:]+'(.*)', views.serve, {'document_root': settings.MEDIA_SITE_ROOT, 'show_indexes': True}),
                            ]

    #To use the view with a different local development server, 
    #add the following helper function that'll do this for you to the end of 
    #your primary URL configuration

    from django.contrib.staticfiles.urls import staticfiles_urlpatterns
    urlpatterns += staticfiles_urlpatterns()
