# term-com
A server-client program written in c
# how to compile
To compile the server run
```make server```
To compile the client run
```make client```
To compile both
```make all```
To clean the bin directory run
```make clean```

# how to run the server
The server accepts two flags: the port the server is going to be running on and maxium pending clients. 
To set the port type -p or --port argument followed by the port number you would want the server to run. If no port is specified server will run on port 0. 
To set the maximum pending clients use -mp or --maxPen arguments followed by the number of maximum pending clients. If nothing is specified the server will accept 32 as is value
# how to run the client
The client accepts two flags: the ip its going to connect to and the port. 
To specify the ip use -ip flag followed by the ip you would like to connect. If no ip is specified it will default to localhost.
To set the port use --port or -p followed by the port you would like to connect. If no port is specified the system will default to port 8080 
# status
I would not reccomend anyone to use this as it is right now. It is probably not very stable and easy to use. I am uploading this project to synchronize it with my pc and laptop.

