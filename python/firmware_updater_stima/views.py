from django.shortcuts import render
from django.http import HttpResponse
from django.http import FileResponse
from django.contrib.auth.hashers import check_password, make_password
import django.utils.timezone
from .models import Firmware
import os
import hashlib
import json
from rmap.stations.models import Board,BoardFirmwareMetadata
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
If version and revision string argument is given, it will be sent to the server. Server side script can use this to check if update should be performed.<br>
Server side script can respond as follows:<br>
- response code 200, and send the firmware image,<br>
- or response code 304 to notify the module that no update is required.<br>
<br>
Add to this url path the name of the firmware you want to update with the last version.<br>
The version and revision on the module have to be set to integer value.<br>
    '''
    return HttpResponse(howto)


def update(request,name):

    board_mac    = request.META.get('HTTP_X_STIMA4_BOARD_MAC')

    if board_mac is None:
        print("403 Forbidden only for stima4 updater! (missed board_mac")
        return HttpResponse("403 Forbidden only for STIMA4 updater! (missed X_STIMA4_BOARD_MAC in header)",status=403)
        

    if request.META.get('HTTP_USER_AGENT') != 'STIMA4-http-Update':
        print("403 Forbidden only for stima4 updater! (missed or wrong HTTP_USER_AGENT")
        return HttpResponse("403 Forbidden only for stima4 updater! (missed or wrong HTTP_USER_AGENT)",status=403)

    try:
        firmware=Firmware.objects.filter(firmware__name=name,active=True,).order_by('version','revision').reverse()[0]
    except:
        print(' 500 no firmware available on server for stima4 firmware name')
        return HttpResponse(' 500 no firmware available for stima4 firmware name',status=500)

    try:
        #check date (version)
        swversion=json.loads(request.META.get('HTTP_X_STIMA4_VERSION'))
        version = (swversion["version"])
        revision = (swversion["revision"])
    except:
        traceback.print_exc()
        print(' 300 No valid version/revision!')
        return HttpResponse(' 300 No valid version/revision!',status=300)

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
        print("no board slug in version")        

    try:
        myboard = Board.objects.get(slug=swversion["bslug"]
                                    ,stationmetadata__slug=swversion["slug"]
                                    ,stationmetadata__user__username=swversion["user"])

        if hasattr(myboard, 'boardfirmwaremetadata'):
            if not myboard.boardfirmwaremetadata.mac:
                print("update missed mac in firmware updater")
                myboard.boardfirmwaremetadata.mac=make_password(board_mac)
                myboard.boardfirmwaremetadata.save(update_fields=['mac',])
        else:
            print("add firmware metadata to board")
            bfm=BoardFirmwareMetadata(board=myboard,mac=make_password(board_mac))
            myboard.boardfirmwaremetadata=bfm
            myboard.boardfirmwaremetadata.save()


        if check_password(board_mac, myboard.boardfirmwaremetadata.mac):
            print("update firmware metadata")

            myboard.boardfirmwaremetadata.swversion=str(version)+"."+str(revision)
            myboard.boardfirmwaremetadata.swlastupdate=django.utils.timezone.now()            
            myboard.boardfirmwaremetadata.save()
        else:
            print("WARNING! mac mismach in firmware updater")
            
    except:
        print("user/station/board not present on DB; ignore it")
        traceback.print_exc()

    update=False        
    if version < firmware.version:
        update=True
    if version == firmware.version and revision < firmware.revision:
        update = True

    if not update:
        print(' 304 No new firmware')
        return HttpResponse(' 304 No new firmware',status=304)
    
    mymd5=md5(firmware.file.path)
    mysize=os.path.getsize(firmware.file.path)

    response=FileResponse(open(firmware.file.path,'rb'))    
    response['Content-Disposition'] = 'attachment; filename="firmware.bin"'
    response['Content-Type']= 'application/octet-stream'
    response['Content-Length']=mysize
    #https://tools.ietf.org/html/rfc1864#section-2
    response['x-MD5']= mymd5 
    response['version'] = firmware.version
    response['revision'] = firmware.revision
    
    print("send new firmware")
    return response
    
