import socket

UDP_IP = "127.0.1.1"
UDP_PORT = 5690

sock = socket.socket(socket.AF_INET6, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind(("", UDP_PORT))

while True:
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    message=data.encode('hex')
    header = message[0:4]
    record1 = message[4:40]
    record2 = message[40:76]
    record3 = message[76:112]
    records=[record1,record2,record3]
    for r in records:
      print "device id: %s; seq: %s%s, hops: %s, HR: %s \n" %(r[0:16],r[18:20],r[16:18],r[22:24],r[24:26])
    print "-----------------------------------------------\n"
