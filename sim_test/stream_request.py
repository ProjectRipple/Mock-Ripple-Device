import socket,struct,sys,time

#stream_source = 'aaaa:0000:0000:0000:c30c:0000:0000:0002'#or c30c
stream_source = 'aaaa:0000:0000:0000:0212:7402:0002:0202'
stream_destination = "AAAA0000000000000000000000000001"
UDP_PORT = 5688
renewals=3#number of times for renewal
renewal_period = 7.0#7 seconds between each renewal
frames_per_request = "30"
stream_type="3"#3 for ECG or 5 for Respiration, 0 for vitalcast record

#MYTTL = 3 # Increase to reach other networks

#addrinfo = socket.getaddrinfo(stream_source, None)[0]
#ttl_bin = struct.pack('@i', MYTTL)
#sock = socket.socket(addrinfo[0],socket.SOCK_DGRAM) # UDP

sock = socket.socket(socket.AF_INET6,socket.SOCK_DGRAM) # UDP
#sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, ttl_bin)
sock.bind(("", UDP_PORT))

request = "d2" + "1" + stream_type  +  stream_destination + frames_per_request
# for i in range(renewals):
#   sock.sendto(request.decode('hex'),(stream_source,UDP_PORT))
#   next_request = time.time()+7.0
#   while next_request > time.time():
#     data, addr = sock.recvfrom(1024)
#     print data
#     print "\n"
print "\n"
sock.sendto(request.decode('hex'),(stream_source,UDP_PORT))
while True:
  data, addr = sock.recvfrom(1024)
  for each in range(100):
    ecg_val=data[6+each*2:8+each*2]
    print struct.unpack('!H',ecg_val)[0]
    #print ecg_val.encode('hex')
  #print data.encode('hex')
