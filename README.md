# SimpleWebServer

V00796911
Lab Section: B01

Specification for this project: [a relative link](p1.pdf)

How it works:
1) check number of arguments when the program was invoked
2) change current dir to the given dir
3) open a socket
4) setsocketopt()
5) bind the socket
5) use select() to check for input (keyboard or client)
7) if input is from keyboard
   if input is 'q' then close the socket and exit
   otherwise do nothing
8) if input is from client then break up the request by spaces using strtok()
     check if the http method, version, and end of http header is valid
       if not then 400 bad request
     if the request is valid 
       then check if the file exist then read the whole file and send 200 OK and break the data into small size and send to the client 
       otherwise 404 not found
     otherwise 400 bad request

Sample Client Request:
echo -e -n "GET /index.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080

'/' is the directory given when the server is invoke
Example:
./sws 8080 /tmp/www
so '/' is /tmp/www

testfile.sh
expect result
1. HTTP/1.0 200 OK
2. HTTP/1.0 200 OK
3. HTTP/1.0 400 Bad Request
4. HTTP/1.0 400 Bad Request
5. HTTP/1.0 404 Not Found
6. HTTP/1.0 404 Not Found
7. HTTP/1.0 200 OK