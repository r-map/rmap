from datetime import datetime

from django.views.decorators.csrf import csrf_exempt
from django.http import HttpResponse
from django.views.decorators.cache import never_cache

import paho.mqtt.publish

@never_cache
@csrf_exempt
def publish(request):
    """Rewriting of php2mqtt."""

    response = HttpResponse()


    if "time" in request.GET:
        response.write(datetime.utcnow().strftime('%y/%m/%d,%H:%M:%S+00\n'))

    try:
        topic = request.GET["topic"]
    except KeyError:
        response.write("please set topic")
        #response.status_code=500
        return response

    try:
        payload = request.GET["payload"]
    except KeyError:
        response.write("please set payload")
        #response.status_code=500
        return response

    try:
        user = request.GET["user"]
    except KeyError:
        user = None
        response.write("user not set\n")

    try:
        password = request.GET["password"]
    except KeyError:
        password = None
        response.write("password not set\n")

    try:
        auth=None
        if user is not None:
            auth={"username": user, "password": password}
        paho.mqtt.publish.single(
            topic, payload=payload, qos=1, hostname="localhost", port=1883,
            client_id="http2mqtt Client",auth=auth)

    except Exception as e:
        response.write(str(e))
        #response.write("MQTT error")
        response.status_code=500
        return response

    response.write("OK")

    return response
