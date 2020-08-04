1) proxy.c is the file with the proxy code, and proxy_parse.c is the library for parsing the input string
got from user. Client.c is the user program.

2)'make' will compile proxy.c and proxy_parse.c.
Note that in proxy.c, you have to give the port no. of proxy from outside as a command line arguement. Say, you give it as 12346.
Then, in that case, you have to change line 12 of Client.c as: # define PORT "12346".

3) Client.c has to be compiled separately, as: "gcc Client.c -o Client". To run this, open 2 terminals.If your proxy port is 6666, then, in one terminal,
write:
 ./proxy 6666
Only then, in another, write:
./Client localhost

Here, I am running proxy on same laptop as client, so I wrote localhost; else you have to write IP of machine where proxy.c is running.
You will get output as an HTML page in Client.

4) In line 129 of proxy.c, in: "while(count<5)", the 5 is the "maximum no of connections" to the server you can make. Change this 5 to any number you want.

5) The string to be sent from Client.c to proxy, is hard coded in. In line 83 of Client.c, you should write the
HTTP request line. Thus, in a way, my 1 Client program can make only make 1 request to Proxy, and then exits.
But this is not a problem, as if you want to make another request, just change this request line in the same Client
program to the request line you want, open a new terminal(if you want for comfort, else you can continue in same terminal),
recompile Client.c, and run it. You'll get output.
One disadvantage of hard-coding request line is that same client cant make multiple requests to proxy, and separate requests
count as separate clients, quickly consuming our upper limit of "maximum number of connections" (5 in my case).
But advantage is that you dont have to write a new Client program with new request line separately. You can just raise "maximum number of connections", in any case. 
