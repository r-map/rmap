#!/usr/bin/env python
import pika
import sys

user="rmap"
password="rmap"
host="localhost"
queue="dballe"
routing_key="ciao"

credentials=pika.PlainCredentials(user, password)

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials))
channel = connection.channel()

#channel.queue_declare(queue=queue)

print ' [*] Waiting for messages. To exit press CTRL+C'

def callback(ch, method, properties, body):
    print " [x] Received %r" % (body,)

    out_file = open("outfile","w")
    out_file.write(body)
    out_file.close()
    sys.exit(0)

channel.basic_consume(callback,
                      queue=queue,
                      no_ack=True)

channel.start_consuming()
