echo -e -n "GET / HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "Get /index.html HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "GET / HTTP/1.1\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "Post / HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "GET /helloworld HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "GET /gnu/helloworld HTTP/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
echo -e -n "GET /gnu/main.html Http/1.0\r\n\r\n" | nc -u -s 192.168.1.100 10.10.1.100 8080 &
sleep 1
