#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#Author:        Moris Pozzati <pozzati@meeo.it>
#Description:   Manage contact messages and email
#Changelog:     Wed Oct 25 12:00:00 CET 2017
#               Fist version
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


from django.http import HttpResponse
import json
from django.contrib.auth.models import User
from django.core.mail import send_mail

import rmap.settings as settings

#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
#         Manage email from the homepage contact form 
#:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
def contacts( request ):
    post = request.POST
    subject   = "[ RAINBO ] Contact message from %s" %( post[ 'name' ] )
    message   = """ 
You have received a new message from RAINBO landing page contact form.

Here are the details:

Name: %s

Email: %s

Phone: %s

Message:
%s

%s
""" %( post[ 'name' ], post[ 'email' ], post[ 'phone' ], post[ 'message' ], settings.RAINBO_MESSAGE_SIGNATURE )
    receivers = get_admin_mail()
    sender    = settings.DEFAULT_FROM_EMAIL
    send_mail( subject, message, receivers, sender,fail_silently=False )
    
    result = json.dumps( { 'error_message' : None } )
    
    return HttpResponse( result, content_type='application/json' )

def get_admin_mail():
    mail_list = []
    for mail in User.objects.filter( is_superuser=True ).values( 'email' ):
         mail_list.append( mail['email'] )
    return mail_list
