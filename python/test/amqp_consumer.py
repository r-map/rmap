#!/usr/bin/env python
import pika
import sys
import ssl

#user="arpae"
#password="arpae2303!!"
#host="meteohub-dev.hpc.cineca.it"
#queue="arpae-output"
#routing_key="meteohub"



user="rmap"
password="div26ic"
host="test.rmap.cc"
queue="rmap..bufr.report_fixed"
routing_key="report_fixed"


credentials=pika.PlainCredentials(user, password)
#context = ssl.create_default_context()

connection = pika.BlockingConnection(pika.ConnectionParameters(
        host=host,credentials=credentials)) #,ssl_options=pika.SSLOptions(context)))
channel = connection.channel()

#channel.queue_declare(queue=queue)

print(' [*] Waiting for messages. To exit press CTRL+C')

def callback(ch, method, properties, body):
    print(" [x] Received %r" % (body,))

    out_file = open("outfile","ab")
    out_file.write(body)
    out_file.close()
    ch.basic_ack(delivery_tag = method.delivery_tag)
    channel.stop_consuming()
    #sys.exit(0)

channel.basic_consume(on_message_callback=callback,
                      queue=queue)

channel.start_consuming()

connection.close()
