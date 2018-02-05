from django.shortcuts import render
from django.http import HttpResponse
from django.http import FileResponse
from models import Firmware
import os
import hashlib
import json
import dateutil.parser

file_full_path="/dati/pat1/eeepc/pat1/Dropbox/Photos/smr/img001.jpg"

def md5(fname):
    hash_md5 = hashlib.md5()
    with open(fname, "rb") as f:
        for chunk in iter(lambda: f.read(4096), b""):
            hash_md5.update(chunk)
    return hash_md5.hexdigest()

def index(request):
    howto='''
    Advanced firmware updater.<br><br>
In the firmware it's possible to point update function to a script at the server.<br>
If version string argument is given, it will be sent to the server. Server side script can use this to check if update should be performed.<br>
Server side script can respond as follows:<br>
- response code 200, and send the firmware image,<br>
- or response code 304 to notify ESP that no update is required.<br>
<br>
Add to this url path the name of the firmware you want to update with the last version.<br>
The version on the ESP have to be set to the date in standard iso format.
    '''
    return HttpResponse(howto)


def update(request,name):

    sta_mac    = request.META.get('HTTP_X_ESP8266_STA_MAC')
    ap_mac     = request.META.get('HTTP_X_ESP8266_AP_MAC' )
    free_space = request.META.get('HTTP_X_ESP8266_FREE_SPACE')
    sketch_size= request.META.get('HTTP_X_ESP8266_SKETCH_SIZE')
    sketch_md5 = request.META.get('HTTP_X_ESP8266_SKETCH_MD5')
    chip_size  = request.META.get('HTTP_X_ESP8266_CHIP_SIZE')
    sdk_version= request.META.get('HTTP_X_ESP8266_SDK_VERSION')

    print "sta_mac,ap_mac,free_space,sketch_size,sketch_md5,chip_size,sdk_version"
    print sta_mac,ap_mac,free_space,sketch_size,sketch_md5,chip_size,sdk_version
    
    if sta_mac is None\
       or ap_mac is None\
       or free_space is None\
       or sketch_size is None\
       or chip_size is None\
       or sdk_version is None :
        #       or sketch_md5 is None\

        return HttpResponse("403 Forbidden only for ESP8266 updater! (header)",status=403)
        

    if request.META.get('HTTP_USER_AGENT') != 'ESP8266-http-Update':
        print "403 Forbidden only for ESP8266 updater!"
        return HttpResponse("403 Forbidden only for ESP8266 updater!",status=403)

    try:
        firmware=Firmware.objects.filter(firmware__name=name,active=True,).order_by('date').reverse()[0]
    except:
        print ' 500 no version for ESP firmware name'
        return HttpResponse(' 500 no version for ESP firmware name',status=500)

    try:
        #check date (version)
        espversion=json.loads(request.META.get('HTTP_X_ESP8266_VERSION'))
        espdate = dateutil.parser.parse(espversion["ver"])
    except:
        print ' 300 No valid version!'
        return HttpResponse(' 300 No valid version!',status=300)

    try:
        print "user: ",espversion["user"]
    except:
        print "no user in version"
    try:
        print "slug: ",espversion["slug"]
    except:
        print "no slug in version"

    if espdate >= firmware.date.replace(tzinfo=None):
        print ' 304 No new firmware'
        return HttpResponse(' 304 No new firmware',status=304)


    mymd5=md5(firmware.file.path)
    mysize=os.path.getsize(firmware.file.path)

    if sketch_md5 == mymd5:
        print ' 304 Not Modified'
        return HttpResponse(' 304 Not Modified',status=304)
    
    response=FileResponse(open(firmware.file.path,'rb'))    
    response['Content-Disposition'] = 'attachment; filename="firmware.bin"'
    response['Content-Type']= 'application/octet-stream'
    response['Content-Length']=mysize
    #https://tools.ietf.org/html/rfc1864#section-2
    response['x-MD5']= mymd5 

    print "send new firmware"
    return response
    
