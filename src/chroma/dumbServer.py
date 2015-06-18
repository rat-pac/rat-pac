import sys
import zmq

context = zmq.Context().instance()

socket =context.socket(zmq.REP)
socket.bind("tcp://*:5554")

def Server():
    print "opened"
    msg = socket.recv()
    print msg, "\n"
    socket.send(msg)

Server()
