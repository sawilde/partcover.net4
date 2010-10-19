
import socket
import struct
import sys
import RcfMessages_pb2
import Person_pb2

errors                      = 0
data                        = ''
port                        = int(sys.argv[1])

#-------------------------------------------------------------------------------
# Send the request.

# Encode the filter header.
filterHeader                = RcfMessages_pb2.FilterHeader()
filterHeader.unfilteredLen  = 0
msg                         = filterHeader.SerializeToString()
msgLen                      = len(msg)
msgLenPacked                = struct.pack('<i', msgLen)
data                        = data + msgLenPacked + msg

# Encode the request header.
request                     = RcfMessages_pb2.RequestHeader()
request.token               = 0;
request.subInterface        = "I_X";
request.fnId                = 1;
request.serializationProtocol = 1;
request.oneway              = False;
request.close               = False;
request.service             = "I_X";
request.rcfRuntimeVersion   = 6;
request.pingBackInterval    = 0;
request.archiveVersion      = 0;
msg                         = request.SerializeToString()
msgLen                      = len(msg)
msgLenPacked                = struct.pack('<i', msgLen)
data                        = data + msgLenPacked + msg

# Encode a single in-parameter.
person1                     = Person_pb2.Person()
person1.id                  = 123
person1.name                = 'Bob'
person1.email               = 'bob@example.com'
msg                         = person1.SerializeToString()
msgLen                      = len(msg)
msgLenPacked                = struct.pack('<i', msgLen)
data                        = data + msgLenPacked + msg

# Length prefix for the entire message.
dataLen                     = len(data)
dataLenPacked               = struct.pack('<i', dataLen)
data                        = dataLenPacked + data

print 'Sending', repr(data)

# Send message.
HOST                        = 'localhost'
PORT                        = port
s                           = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))
s.send(data)

#-------------------------------------------------------------------------------
# Receive the response

# Receive message.
data                        = s.recv(4)
print 'Received', repr(data)
dataLen                     = struct.unpack('<i', data)[0]
data                        = s.recv(dataLen)
print 'Received', repr(data)
s.close()

# Decode the filter header.
pos                         = 0
len                         = struct.unpack('<i', data[pos:pos+4])[0]
msg                         = data[pos+4:pos+4+len]
filterHeader.ParseFromString(msg)

# Decode the response header.
pos                         = pos + 4 + len
len                         = struct.unpack('<i', data[pos:pos+4])[0]
msg                         = data[pos+4:pos+4+len]
response                    = RcfMessages_pb2.ResponseHeader()
response.ParseFromString(msg)

# Decode the out-parameter.
pos                         = pos + 4 + len
len                         = struct.unpack('<i', data[pos:pos+4])[0]
msg                         = data[pos+4:pos+4+len]
person2                     = Person_pb2.Person()
person2.ParseFromString(msg)

print 'Response type: ', response.responseType
if response.responseType == 2:
	print 'What: ', response.responseException.what	

# Check that person1 == person2
if person1.SerializeToString() != person2.SerializeToString():
	errors = errors + 1

print 'Errors detected: ', errors

sys.exit(errors)
