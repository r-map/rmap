#!/usr/bin/env python
import pika

user="rmap"
password="rmap"
host="localhost"
exchange="rmap"
routing_key="ciao"
body='Hello World!'

# Legge un file.
in_file = open("infile","r")
body = in_file.read()
in_file.close()


credentials=pika.PlainCredentials(user, password)
properties=pika.BasicProperties(
    user_id= user,
    delivery_mode = 2, # persistent
)

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
channel = connection.channel()

#channel.queue_declare(queue=queue)

channel.basic_publish(exchange=exchange,
                      routing_key=routing_key,
                      body=body,
                      properties=properties)

print " [x] Sent ",body
connection.close()
