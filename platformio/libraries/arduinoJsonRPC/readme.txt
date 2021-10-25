JsonRPC is a software library for arduino that implements (a small part of) the
JSON-RPC protocol (see http://json-rpc.org/).

For the interpretation of json objects, the library makes use of the aJson 
library (https://github.com/interactive-matter/aJson). A copy of the library
can also be found in the aJson directory.

Currently only a subset of version 1.0 of the protocol is supported.
Only a proper request message with method and params keys will be
interpreted, the id will be ignored for now. If a method by the same name
is registered with the library, the method will be called with exactly one
argument of type aJsonObject*.