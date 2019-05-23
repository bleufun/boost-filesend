# Filesend
A simple one-way file transfer utility over tcp sockets, that closes connections when specified timeout has been reached, based on boost::asio framework. A single server can manage multiple connections from clients, saving received files in single download directory.

Requirements
- boost 1.69
- clang >= 8.0
- bash (for building scripts)

Invoke
```
file-server --help
file-client --help
```
for information about use,
```
file-server --download-dir <directory>
```
for running file server,
```
file-client --address <server-address> --port <port> <file>
```
for transfering file to server
