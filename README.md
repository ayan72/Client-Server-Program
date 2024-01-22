# Client-Server-Program

DESIGN OVERVIEW
• The program can be invoked as a client and as a server. To invoke the program as a server use “./a3w23 -s portNum” and to invoke the program as a client use “a3w23 -c idNumber inputFile serverAddress portNumber” where idNumber can be 1, 2 or 3; inputFile is a3-ex1.dat; serverAddress is the IP address in the form of symbolic name; portNum is 9145 where 145 is the last three digits of my student ID to avoid conflict. I have tried to check for invalid inputs but all the cases may not be covered so follow the above.
• The input file was parsed and the client IDs, Packet type, and object names were extracted using stringstream. The empty lines and commented lines are skipped and I have assumed a single space between each of the input. The input file should be in the format of the assignment description because I have not dealt with invalid lines in the input file. The client id matching the id entered while invoking the program are parsed. The other lines are ignored.
• To implement the packets, I have used a struct with a message and a type. The type was implemented using enum and the message is a union of all types of message which can be encountered. I have created separate functions to compose these messages and separate functions to send and receive the packets. Most of the packet types have a message though some of them are empty.
• For the client loop, I have created a socket and connected the socket. After that, I parsed through the input file and sent and received the required packets depending on the line. The information was also printed as I received or sent the packets.
• For the server loop, I created the socket, then I used bind to turn it into a managing socket. And I used listen. Then I used poll function to handle I/O from the keyboard and the sockets in a non-blocking manner. The user can enter list and quit from the keyboard. The poll function also handles the clients and the packages which are received. The server also detects any closed connections as well.
• The executable for the program can be made using the Makefile. Use “make a3w23” to make the executable of the program and run the client-server using the correct command for each of them. Run the server in a separate terminal. Then run the clients in a different terminal.
• The IP address should be the same as the lab machine which you are connected to.

Makefile
• “make all” creates the executable of all the files
• “make a3w23” creates the executable for a3w23.cpp
• “make clean” removed all the executables and object files
• “make tar” compresses the desired files

ACKNOWLEDGEMENT
1. Poll program demonstrated in the lectures by Ehab Elmallah.
2. Concurrent Client-server program demonstrated in the class by Ehab Elmallah.
3. sockMsg.cc program on eClass by Ehab Elmallah
4. http://webdocs.cs.ualberta.ca/~cmput379/W23/379only/sockMsg.cc
5. https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
6. https://linux.die.net/man/2/setsockopt
7. https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-bind-bind-name-socket
8. https://linuxhint.com/use-poll-system-call-c/
9. https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/
10. https://www.tutorialspoint.com/how-to-use-enums-in-cplusplus
11. Linux Man Pages
12. Advanced Programming in Unix Environment
