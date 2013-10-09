import socket,struct,sys,time

stream_source = 'aaaa:0000:0000:0000:c30c:0000:0000:0002'
UDP_PORT = 5688
sock = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM)
sock.bind(("",5688))
sock.sendto("hello!",(stream_source,UDP_PORT))

