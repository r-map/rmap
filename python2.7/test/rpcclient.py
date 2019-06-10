# create JSON-RPC client
import rmap.jsonrpc as jsonrpc
server = jsonrpc.ServerProxy(jsonrpc.JsonRpc20(radio=True,notification=True), jsonrpc.TransportTcpIp(addr=("127.0.0.1", 31415)))

# call a remote-procedure (with positional parameters)
result = server.echo("hello world")
