 Alias /download /var/www/html/download
 Alias /repo     /var/www/html/repo
 Alias /showroom /var/www/html/showroom
 Alias /arkiweb  /var/www/html/arkiweb

 SecRuleEngine On

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
    Allow from <insert IP of data-ingestion machine>    
 </Location>

ErrorDocument 503 "Service Unavailable"
