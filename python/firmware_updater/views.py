from django.shortcuts import render
from django.http import HttpResponse
from django.http import FileResponse
from .models import Firmware
import os
import hashlib
import json
import dateutil.parser
from rmap.stations.models import Board,BoardFirmwareMetadata
import django.utils.timezone
from django.contrib.auth.hashers import check_password, make_password
import traceback

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
- or response code 304 to notify the module that no update is required.<br>
<br>
Add to this url path the name of the firmware you want to update with the last version.<br>
The version on the module have to be set to the date in standard iso format.<br>
If you change the hardware in the module you need to recreate the station on the server becouse the MAC of the board is saved on server side.
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

    print("sta_mac,ap_mac,free_space,sketch_size,sketch_md5,chip_size,sdk_version")
    print(sta_mac,ap_mac,free_space,sketch_size,sketch_md5,chip_size,sdk_version)
    
    if sta_mac is None\
       or ap_mac is None\
       or free_space is None\
       or sketch_size is None\
       or chip_size is None\
       or sdk_version is None :
        #       or sketch_md5 is None\

        return HttpResponse("403 Forbidden only for ESP8266 updater! (header)",status=403)
        

    if request.META.get('HTTP_USER_AGENT') != 'ESP8266-http-Update':
        print("403 Forbidden only for ESP8266 updater!")
        return HttpResponse("403 Forbidden only for ESP8266 updater!",status=403)

    try:
        firmware=Firmware.objects.filter(firmware__name=name,active=True,).order_by('date').reverse()[0]
    except:
        print(' 500 no version for ESP firmware name')
        return HttpResponse(' 500 no version for ESP firmware name',status=500)

    try:
        #check date (version)
        swversion=json.loads(request.META.get('HTTP_X_ESP8266_VERSION'))
        swdate = dateutil.parser.parse(swversion["ver"])
    except:
        print(' 300 No valid version!')
        return HttpResponse(' 300 No valid version!',status=300)

    try:
        print("user: ",swversion["user"])
    except:
        print("no user in version")
    try:
        print("slug: ",swversion["slug"])
    except:
        print("no slug in version")

    try:
        print("boardslug: ",swversion["bslug"])
    except:
        print("no board slug in version; set default")
        swversion["bslug"]="default"

    try:
        myboard = Board.objects.get(slug=swversion["bslug"]
                                    ,stationmetadata__slug=swversion["slug"]
                                    ,stationmetadata__ident__username=swversion["user"])

        if hasattr(myboard, 'boardfirmwaremetadata'):
            if not myboard.boardfirmwaremetadata.mac:
                print("update missed mac in firmware updater")
                myboard.boardfirmwaremetadata.mac=make_password(sta_mac)
                myboard.boardfirmwaremetadata.save(update_fields=['mac',])
        else:
            print("add firmware metadata to board")
            bfm=BoardFirmwareMetadata(board=myboard,mac=make_password(sta_mac))
            myboard.boardfirmwaremetadata=bfm
            myboard.save()
        if check_password(sta_mac, myboard.boardfirmwaremetadata.mac):
            print("update firmware metadata")
            myboard.boardfirmwaremetadata.swversion=swversion["ver"]
            myboard.boardfirmwaremetadata.swlastupdate=django.utils.timezone.now()            
            myboard.boardfirmwaremetadata.save()
        else:
            print("WARNING! mac mismach in firmware updater")
            
    except:
        print("user/station/board not present on DB; ignore it")
        traceback.print_exc()

    if swdate >= firmware.date.replace(tzinfo=None):
        print(' 304 No new firmware')
        return HttpResponse(' 304 No new firmware',status=304)
    
    mymd5=md5(firmware.file.path)
    mysize=os.path.getsize(firmware.file.path)

    if sketch_md5 == mymd5:
        print(' 304 Not Modified')
        return HttpResponse(' 304 Not Modified',status=304)
    
    response=FileResponse(open(firmware.file.path,'rb'))    
    response['Content-Disposition'] = 'attachment; filename="firmware.bin"'
    response['Content-Type']= 'application/octet-stream'
    response['Content-Length']=mysize
    #https://tools.ietf.org/html/rfc1864#section-2
    response['x-MD5']= mymd5 

    print("send new firmware")
    return response
    
