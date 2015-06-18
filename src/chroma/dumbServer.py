import sys
import zmq
import ratchromadata_pb2

context = zmq.Context().instance()

socket =context.socket(zmq.REP)
socket.bind("tcp://*:5554")

def Server():
    print "opened"
    msg = socket.recv()
    print ParseFromString(msg), "\n"
    socket.send(msg)

Server()
