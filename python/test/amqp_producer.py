#!/usr/bin/env python
import pika
import ssl
import logging

user="rmap"
password="div26ic"
host="rmap.arpae.it"
exchange="rmap.mqtt.bufr.sample_mobile"
routing_key="ciao"
body='Hello World!'


#logging.basicConfig(level=logging.INFO)

# Legge un file.
#in_file = open("infile","r")
#body = in_file.read()
#in_file.close()

#context = ssl.SSLContext(ssl.PROTOCOL_TLSv1_2)
context = ssl.create_default_context()
context.verify_mode = ssl.CERT_REQUIRED

credentials=pika.PlainCredentials(user, password)
properties=pika.BasicProperties(
    user_id= user,
    delivery_mode = 2, # persistent
)

connection = pika.BlockingConnection(pika.ConnectionParameters(
    ssl_options=pika.SSLOptions(context),
    host=host, port=5671,credentials=credentials))
channel = connection.channel()

#channel.queue_declare(queue=queue)

channel.basic_publish(exchange=exchange,
                      routing_key=routing_key,
                      body=body,
                      properties=properties)

print(" [x] Sent ",body)
connection.close()
