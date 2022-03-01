import socket
import sys
import os

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
sock.connect((sys.argv[1], 8888))
sock.send(bytes(os.getlogin() + '\n', encoding='utf-8'))
message = sock.recv(4096)
print(str(message, 'utf-8'))
sock.close()
