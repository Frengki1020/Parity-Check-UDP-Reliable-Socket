<p><h4> # Parity-Check-UDP-Reliable-Socket</h4></p>
<p><h5>1. make Directory SERVER and CLIENT</h5></p>
mkdir SERVER CLIENT
<p><h5>2. make file server.c and file(.txt) in directory SERVER</h5></p>
<p>nano testing.txt (make some text)</p>
<p>nano server.c</p>
<p>copy and paste program server.c</p>
<p><h5>3. make file client.c in directory CLIENT</h5></p>
<p>nano server.c</p>
<p>copy and paste program client.c</p>
<p><h5>4. Compile Program Server and Client</h5></p>
<p>=> gcc -o client client.c -lm (-lm untuk header math->pow)</p>
<p>=> gcc -o server server.c</p>
<p><h5>5. Run program server</p>
<p>./server</p>
<p>program server will create file testing.txt.txt (should be file binary (0,1)</p>
<p>and server send this file to client</p>
<p><h5>6. Run program client</h5></p>
<p>./client [ip_server] [filename_server]</p>
<p>./client 10.10.10.10 testing.txt</p>
<p>client convert file biner to ASCII. client get the file (testing.txt)</p>

