# UnixDomainSockets
## Purpose:
This client/server application demonstrates inter-process communication using UNIX Domain Sockets. The server component accepts a parameter that acts as a key to encrypting content sent from the client. Upon encryption, the server sends the client the encrypted content back.

## Installing:

### Obtaining
```sh
git clone https://github.com/ClownCrest/UnixDomainSockets
```

### Building
```sh
gcc server.c -o server
gcc client.c -o client
```

### Running
```sh
./server <shift>
./client <filename>
```
  
## Examples
```sh
./server 10
./client hello.txt
```
