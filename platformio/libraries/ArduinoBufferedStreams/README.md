Loopback Stream
===============

This is a very simple implementation of Arduino's Stream class, where all data written with write() can be read back with read()

It can be used to easily add a buffering layer to communications.

Piped Streams
=============

PipedStreams come in pairs. Anything written to one of them can be read back on the other.

It can be used to easily implement the communication between multiple components in a Serial-like APIs  (Maybe Socket-like API?)
