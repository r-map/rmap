from datetime import datetime

from django.views.decorators.csrf import csrf_exempt
from django.http import HttpResponse

import paho.mqtt.publish


@csrf_exempt
def publish(request):
    """Rewriting of php2mqtt."""
    response = ""

    if "time" in request.GET:
        return HttpResponse(
            request, datetime.utcnow().strftime('%y/%m/%d,%H:%M:%S+00\n'),
        )

    try:
        topic = request.GET["topic"]
    except KeyError:
        return HttpResponse(request, "please set topic", status_code=500)

    try:
        payload = request.GET["payload"]
    except KeyError:
        return HttpResponse(request, "please set payload", status_code=500)

    try:
        user = request.GET["user"]
    except KeyError:
        user = None
        response += "user not set\n"

    try:
        password = request.GET["password"]
    except KeyError:
        password = None
        response += "password not set\n"

    try:
        paho.mqtt.publish.single(
            topic, payload=payload, qos=0, hostname="localhost", port=1883,
            client_id="Python MQTT Client",
            auth={"username": user, "password": password},
    )
    except Exception as e:
        response += str(e)
        return HttpResponse(request, response, status_code=500)

    response += "okay\n"

    return HttpResponse(request, response)
