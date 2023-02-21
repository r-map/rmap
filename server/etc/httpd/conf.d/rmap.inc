 Alias /download /var/www/html/download
 Alias /repo     /var/www/html/repo
 Alias /showroom /var/www/html/showroom
 Alias /arkiweb  /var/www/html/arkiweb
 
 #Alias /static/admin /usr/lib/python2.7/site-packages/django/contrib/admin/static/admin
 #<Directory "/usr/lib/python2.7/site-packages/django/contrib/admin/static/admin">
 #	    Require all granted
 #</Directory>
 
 Alias /static   /usr/share/rmap/static
 <Directory /usr/share/rmap/static>
        Require all granted
        SetHandler None
 </Directory>
 
 Alias /media /usr/share/rmap/media
 <Directory /usr/share/rmap/media>
        Require all granted
        SetHandler None
 </Directory>
 
 WSGIScriptAlias / /usr/bin/rmap.wsgi
 #WSGIImportScript /usr/bin/rmap.wsgi process-group=%{GLOBAL} application-group=%{GLOBAL}
 #WSGIPythonPath /path/to/mysite.com
 
 #WSGIDaemonProcess rmap processes=5 threads=5
 #WSGIDaemonProcess rmap
 #WSGIProcessGroup rmap
 
 <Directory /usr/bin>
 <Files rmap.wsgi>
 Require all granted
 </Files>
 </Directory>
 
 <Location /auth>
    Order Deny,Allow
    Deny from all
    Allow from 127.0.0.1
 </Location>

#<IfModule mod_evasive24.c>
#<Location /borinud>
#    # This is the threshhold for the number of requests for the same page (or
#    # URI) per page interval.  Once the threshhold for that interval has been
#    # exceeded, the IP address of the client will be added to the blocking
#    # list.
#    #DOSPageCount        2
#
#    # The interval for the page count threshhold; defaults to 1 second
#    # intervals.
#    #DOSPageInterval     900
#
#    # The blocking period is the amount of time (in seconds) that a client will
#    # be blocked for if they are added to the blocking list.  During this time,
#    # all subsequent requests from the client will result in a 403 (Forbidden)
#    # and the timer being reset (e.g. another 10 seconds).  Since the timer is
#    # reset for every subsequent request, it is not necessary to have a long
#    # blocking period; in the event of a DoS attack, this timer will keep
#    # getting reset.
#    DOSBlockingPeriod   3600
# </Location>
#</IfModule>

SecRuleEngine On

#The first SecAction initializes the state, in this case by IP address.
#The second SecAction deprecates the counter by 1 every 60 second.
#This is setting the base rate of our rate limit
#Then the Header definition ensures a header is set whenever a request
#is rate limited, giving a hint to the client that they shouldn’t try
#again for 10 seconds. This is obviously just a guide and a lot of
#clients don’t implement it (and it’s really only valid on a 503 status
#anyway) so it’s a little bit of wishful thinking really.
#Then we define a neat ErrorDocument for the 509 response to give a better clue to the client about what is happening.


<Location /borinud>

  # whitelist localhost
  #SecRule REMOTE_ADDR "@contains 127.0.0.1" "id:1,phase:1,nolog,allow,ctl:ruleEngine=Off"
  SecRule REMOTE_ADDR "^127.0.0.1$" nolog,allow,id:1

  # initialise the state based on ip address
  SecAction id:2,initcol:IP=%{REMOTE_ADDR}

  # set the base rate to one per 15 minutes
  SecAction id:3,deprecatevar:IP.CALLS_LIMIT=1/900

  # if greater then burst_rate_limit then pause set RATELIMITED var and then return 503
  SecRule IP:CALLS_LIMIT "@gt 60" "id:4,phase:2,pause:300,deny,status:503,setenv:RATELIMITED,skip:1"

  # if above rule doesnt match increment the count
  SecAction id:5,setvar:IP.CALLS_LIMIT=+1

  # set a header when ratelimited
  Header always set Retry-After "10" env=RATELIMITED

</Location>


<Location /auth>

  # whitelist localhost
  #SecRule REMOTE_ADDR "@contains 127.0.0.1" "id:1,phase:1,nolog,allow,ctl:ruleEngine=Off"
  SecRule REMOTE_ADDR "^127.0.0.1$" nolog,allow,id:1

</Location>

ErrorDocument 503 "Service Unavailable"
