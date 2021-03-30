# Parity-Check-UDP-Reliable-Socket
1. make Directory SERVER and CLIENT
mkdir SERVER CLIENT
2. make file server.c and file(.txt) in directory SERVER
nano testing.txt (make some text)
nano server.c
copy and paste program server.c
3. make file client.c in directory CLIENT
nano server.c
copy and paste program client.c
4. Compile Program Server and Client
=> gcc -o client client.c -lm (-lm untuk header math->pow)
=> gcc -o server server.c
5. Run program server
./server
program server will create file testing.txt.txt (should be file binary (0,1)
and server send this file to client
6. Run program client
./client [ip_server] [filename_server]
./client 10.10.10.10 testing.txt
client convert file biner to ASCII. client get the file (testing.txt)

