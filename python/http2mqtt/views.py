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
            datetime.utcnow().strftime('%y/%m/%d,%H:%M:%S+00\n'),
        )

    try:
        topic = request.GET["topic"]
    except KeyError:
        return HttpResponse("please set topic", status=500)

    try:
        payload = request.GET["payload"]
    except KeyError:
        return HttpResponse("please set payload", status=500)

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
        auth=None
        if user is not None:
            auth={"username": user, "password": password}
        paho.mqtt.publish.single(
            topic, payload=payload, qos=1, hostname="localhost", port=1883,
            client_id="http2mqtt Client",
            auth=None,
        )
    except Exception as e:
        response += str(e)
        return HttpResponse(response, status=500)

    response += "okay\n"

    return HttpResponse(response)
