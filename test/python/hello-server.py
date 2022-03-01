
import socket
import time

serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
serversocket.bind(('', 8888))
serversocket.listen(5)

while True:
    (clientsock, address) = serversocket.accept()
    data = clientsock.recv(4096)
    datatime = time.asctime() + '\n'
    clientsock.send(bytes("Hello " + str(data, 'utf-8'), encoding='utf-8'))
    clientsock.send(bytes("My time is " + datatime, encoding='utf-8'))
    clientsock.close()
