#include <unistd.h>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdarg>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <fcntl.h>
#include <errno.h>
#include <fstream> 
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/times.h>
#include <poll.h>

#include <string>
#include <vector>
using namespace std;

#define MAXWORD 32
#define MAXCLIENT 3

//https://www.tutorialspoint.com/how-to-use-enums-in-cplusplus
//using enum for all the types of packets 
typedef enum {HELLO,PUT,GET,DELETE,GTIME,TIME,OK,ERROR} KIND;

char KINDNAME[][MAXWORD] = {"HELLO","PUT","GET","DELETE","GTIME","TIME","OK","ERROR"};

//http://webdocs.cs.ualberta.ca/~cmput379/W23/379only/sockMsg.cc

// creating the message struct for different packets
typedef struct {
    int cId;
} MSG_HELLO;

typedef struct {
    char object[MAXWORD];
} MSG_PUT;

typedef struct {
    char object[MAXWORD];
} MSG_GET;

typedef struct {
    char object[MAXWORD];
} MSG_DELETE;

typedef struct {
} MSG_GTIME;

typedef struct {
    float time;
} MSG_TIME;

typedef struct {
} MSG_OK;

typedef struct {
    char err[MAXWORD];
} MSG_ERROR;

//creating a unioin of the structs as MSG
typedef union {
    MSG_HELLO mHel;
    MSG_PUT mPut;
    MSG_GET mGet;
    MSG_DELETE mDel;
    MSG_GTIME mGt;
    MSG_TIME mTime;
    MSG_OK mOk;
    MSG_ERROR mErr;
} MSG;

//tms used to calculate the time 
struct tms start_time;
struct tms end_time;
struct tms end_t;

typedef struct { KIND kind; MSG msg; } FRAME;

typedef struct sockaddr  SA;

#define MAXBUF  sizeof(FRAME);


typedef struct {
  KIND type;
  string message;
} Packet;

typedef struct {
  int socket_fd;
  sockaddr_in socket_addr;
  socklen_t socket_addr_len;
} TCP;

typedef struct {
    int sockfd;
    struct sockaddr_in addr;
} ClientInfo;

//vector to store the lines of the get command
vector<string> lines;

//http://webdocs.cs.ualberta.ca/~cmput379/W23/379only/sockMsg.cc

//composing the message of the several packets
MSG compHel (int cId)
{
    MSG  msg;

    memset( (char *) &msg, 0, sizeof(msg) );
    msg.mHel.cId= cId; 
    return msg;
}  

MSG compPut(const char* a){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    strcpy(msg.mPut.object,a);
    return msg;
}

MSG compGet(const char* a){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    strcpy(msg.mGet.object,a); 
    return msg;  
}

MSG compDel(const char* a){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    strcpy(msg.mDel.object,a); 
    return msg;     
}

MSG compGtime(){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    return msg;
}

MSG compTime(float t){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    msg.mTime.time = t;
    return msg;
}

MSG compOK(){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    return msg;
}

MSG compError(){
    MSG msg;
    memset( (char *) &msg, 0, sizeof(msg) );
    return msg;
}

//https://www.geeksforgeeks.org/input-output-system-calls-c-create-open-close-read-write/

//function to recieve an incoming frame.
//Parameters:
//int fd : file descriptor of the socket
//FRAME *framep : frame 
//returns: the length l
int rcv (int fd, FRAME *framep){
    int l;
    FRAME frame;

    memset( (char *) &frame, 0, sizeof(frame) );
    l= read (fd, (char *) &frame, sizeof(frame));
    *framep = frame;
    if (l == 0){
        printf("Recieved frame has zero length\n");
        //exit(1);
    }
    return l;
}

//function to send a frame.
//Parameters:
//int fd : file descriptor of the socket
//KIND kind : the type of fram sent
//MSG *msg : message 
//returns: the length l
int send_frame(int fd, KIND kind, MSG *msg){
    
    FRAME frame;
    memset( (char *) &frame, 0, sizeof(frame) );
    frame.kind = kind;
    frame.msg = *msg;

    int l = write(fd, (char *)&frame, sizeof(frame));
    if (l == 0){
        printf("sent frame has zero length\n");
        //exit(1);
    }
    return l;

}

int main(int argc,char* argv[]){
    //checking for invalid inputs
    if (argc <= 2){
        printf("Incorrect Arguments Entered\n");
        printf("Correct Usage: a3w23 -s portNumber\n");
        printf("a3w23 -c idNumber inputFile serverAddress portNumber\n");
        exit(1);
    }
    //handling case when -c or -s is not entered
    char* mode;
    mode = argv[1];
    if (strcmp(mode,"-c")!=0 && strcmp(mode,"-s")!=0){
        printf("Enter -s or -c to run the server/client\n");
        exit(1);
    }

    //printf("Correct\n");
    //server mode
    if (strcmp(mode,"-s")==0 && argc == 3){
        // clock_t start = times(&start_time);
        int new_sock;
        struct sockaddr_in servSockAddr;
        int addrl;
        addrl = sizeof(servSockAddr);
        int portNum;
        portNum = stoi(argv[2]);
        MSG msg;
        FRAME frame;
        //char buffer[MAXBUF];
        printf("a3w23: do_server\n");
        //https://www.geeksforgeeks.org/tcp-server-client-implementation-in-c/
        //creating the socket
        int serverfd = socket(AF_INET, SOCK_STREAM, 0);
        if (serverfd<0){
            fprintf (stderr, "%s: socket \n", argv[0]);
            exit(1);
        }
        // Set socket options to reuse the address and port
        //https://linux.die.net/man/2/setsockopt
        int opt = 1;
        if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
            perror("setsockopt failed");
            exit(1);
        }
        servSockAddr.sin_family= AF_INET;
        servSockAddr.sin_addr.s_addr= htonl (INADDR_ANY);
        servSockAddr.sin_port= htons (portNum);

        //https://www.ibm.com/docs/en/zos/2.4.0?topic=functions-bind-bind-name-socket
        //binding the socket
        if ( bind( serverfd, (SA *)&servSockAddr, sizeof(servSockAddr) ) < 0 ) {
            // fprintf (stderr, "%s: bind \n", argv[0]);
            perror("bind error\n");
            exit(1);
        }

        if (listen(serverfd,MAXCLIENT)< 0){
            perror("Error listening to the socket\n");
            exit(1);
        }

        //declaring the vairables for poll
        //https://linuxhint.com/use-poll-system-call-c/
        int timeout = 0;
        int N;
        int newSock[MAXCLIENT+1];
        char buf[1024];
        struct pollfd pfd[MAXCLIENT+1];
        struct sockaddr_in  from;
        socklen_t l;
        Packet pkt;

        //keyboard I/O
        pfd[0].fd = STDIN_FILENO;
        pfd[0].events = POLLIN;
        pfd[0].revents = 0;

        //server FD
        pfd[1].fd = serverfd;
        pfd[1].events = POLLIN;
        pfd[1].revents = 0;

        printf("Server is accepting connections from (port= %d)\n",portNum);

        N=1;
        //server loop
        while(true){
            int val;
            val = poll(pfd, N+1, timeout);
            if (val < 0){
                perror("poll has failed\n");
                exit(1);
            }
            if(pfd[0].revents & POLLIN){
                char buf[128];
                int length;
                //reading the stdin from the user
                length = read(STDIN_FILENO, buf, sizeof(buf));
                if (length>0){
                    string cmd(buf,length);

                    if (cmd == "list\n"){
                        printf("Stored object table:\n");
                    }
                    else if (cmd == "quit\n"){
                        printf("quitting\n");
                        exit(0);
                    }
                    else{
                        printf("Enter a valid command (list|quit)\n");
                    }
                } 

            }
            if ( (N < MAXCLIENT+1) && (pfd[1].revents & POLLIN) ) {
           // accept a new connection
	        l= sizeof(from);
	        newSock[N]= accept(pfd[0].fd, (SA *) &from, &l);

            pfd[N+1].fd= newSock[N];
	        pfd[N+1].events= POLLIN;
	        pfd[N+1].revents= 0;
	        N++;
            }

            //polling the clients
            for(int i=2; i <= N; i++){
                if(pfd[i].revents & POLLIN){
                    int len = rcv(pfd[i].fd, &frame);
                    char line[1024];
                    //sprintf(line,"Received (src= %s) (HELLO, idNumber= %s)",pkt.message);
                    if (len==0){
                        printf ("server lost connection with client\n");
                        continue;
                    }
                    //handling each incoming frames
                    if (frame.kind == HELLO){
                        //rcv(pfd[i].fd,pkt);
                        msg = compOK();
                        printf("Received (src= %d) (HELLO, idNumber= %d)\n",frame.msg.mHel.cId,frame.msg.mHel.cId);
                        send_frame(pfd[i].fd, OK, &msg);
                        printf(" Transmitted (src= 0) OK\n");
                    }
                    else if(pkt.type == PUT){
                        //rcv(pfd[i].fd,pkt);
                        msg = compOK();
                        printf("Received (src= 1) (PUT= %s)\n",frame.msg.mPut.object);
                        send_frame(pfd[i].fd,OK, &msg);
                        printf(" Transmitted (src= 0) OK\n");
                    }
                    else if(pkt.type == DELETE){
                        //rcv(pfd[i].fd,pkt);
                        msg = compOK();
                        printf("Received (src= 1) (DELETE= %s)\n",frame.msg.mDel.object);
                        send_frame(pfd[i].fd,OK, &msg);
                        printf(" Transmitted (src= 0) OK\n");
                    }
                    else if(pkt.type == GTIME){
                        //rcv(pfd[i].fd,pkt);
                        //clock_t end_ti = times(&end_t);
                        double clock_tick;
                        clock_tick = sysconf(_SC_CLK_TCK);
                        //double real = (end_ti - start)/clock_tick;
                        //msg = compTime(real);
                        printf("Received (src= 1) GTIME\n");
                        send_frame(pfd[i].fd,TIME, &msg);
                        printf("Transmitted (src= 0) (TIME:    1.87)\n");
                    }
                }
            }

        }
        // clock_t end = times(&end_time);
        // double clock_tick;
        // clock_tick = sysconf(_SC_CLK_TCK);
        // //dividing the time by the clock ticks to get the time in seconds
        // double real = (end - start)/clock_tick;
        // double user = (end_time.tms_utime - start_time.tms_utime)/clock_tick;
        // double sys = (end_time.tms_stime - start_time.tms_utime)/clock_tick;

    }


    else if (strcmp(mode,"-c")==0 && argc == 6){
        // client mode
        int portNum;
        portNum = stoi(argv[5]);
        int id;
        id = stoi(argv[2]);
        char* inFile;
        inFile = argv[3];
        string SerAddr;
        SerAddr = string(argv[4]);
        FRAME frame;
        MSG msg;


        struct sockaddr_in s_in;
        struct hostent *hp;
        hp = gethostbyname(SerAddr.c_str());
        memset ((char *) &s_in, 0, sizeof s_in);
        memcpy ((char *) &s_in.sin_addr, hp->h_addr, hp->h_length);
        s_in.sin_family = AF_INET;
        s_in.sin_port = htons(portNum);
        //s_in.sin_addr = htonl(SerAddr.c_str());

        int sfd;
        sfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sfd < 0){
            fprintf (stderr, "%s: socket \n", argv[0]);
            exit(1);
        }

        if (connect(sfd, (SA *) &s_in, sizeof(s_in))<0){
            fprintf (stderr, "%s: bind \n", argv[0]);
            exit(1);
        }
        printf("do_client (idNumber= %d, inputFile= '%s')\n",id,inFile);
        printf("(server= '%s', port= %d)\n",SerAddr.c_str(),portNum);

        msg = compHel(id);
        int l = send_frame(sfd,HELLO,&msg);
        printf("Transmitted (src= %d) (HELLO, idNumber= %d)\n",id,id);
        //Packet res;
        int len = rcv(sfd,&frame);
        if (frame.kind == OK){
            printf(" Received (src= 0) OK\n");
        }
        else {
            printf("Received (src= 0) (ERROR)\n");
        }


        ifstream in(inFile);
        string line;
        while(getline(in,line)){
            //skipping the commented or empty lines
            if(line.empty()||line[0]=='#'){
                continue;
            }
            //removing the trailing newline character
            if (!line.empty() && line[line.size()-1]=='\n'){
                line = line.substr(0, line.size()-1);
            }

            int cId;
            string cmd;
            string obj;
            string content_line;
            int delay;
            //parsing the client id and command
            istringstream iss(line);
            iss >> cId >> cmd;

            if(cId != id){
                //skipping the lines where the client Id does not match the ID entered while invoking the client
                continue;
            }

            if(cmd == "put"){
                iss >> obj;
                lines.clear();
                //cout << "put\n";
                int l;
                while(l < 3){
                    //reading the lines inside {}
                    getline(iss, content_line);
                    if(content_line == "{"){
                        continue;
                    }
                    if(content_line == "}"){
                        break;
                    }
                    lines.push_back(content_line);
                    l++;
                }
                msg = compPut(obj.c_str());
                int len = send_frame(sfd,PUT, &msg);
                printf("Transmitted (src= %d) (PUT:  %s)\n",id,obj.c_str());
                for(int i = 0;i < lines.size();i++){
                    printf("[%d]: '  %s: %s'\n",i,obj.c_str(),lines[i].c_str());
                }
                int le = rcv(sfd,&frame);
                if (frame.kind == OK){
                    printf("Received (src= 0) OK\n");
                }
                else if (frame.kind == ERROR){
                    printf("Received (src= 0) (ERROR)\n");
                }

            }
            else if(cmd == "get"){
                iss >> obj;
                msg = compGet(obj.c_str());
                int len = send_frame(sfd,GET, &msg);
                printf("Transmitted (src= %d) (GET:  %s)\n",id,obj.c_str());
                int L = rcv(sfd,&frame);
                if (frame.kind == OK){
                    printf("Received (src= 0) OK\n");
                }
                else if (frame.kind == ERROR){
                    printf("Received (src= 0) (ERROR)\n");
                }

            }
            else if(cmd == "delete"){
                iss >> obj;
                msg = compDel(obj.c_str());
                int len = send_frame(sfd,DELETE,&msg);
                printf("Transmitted (src= %d) (DELETE:  %s)\n",id,obj.c_str());
                int L = rcv(sfd,&frame);
                if (frame.kind == OK){
                    printf("Received (src= 0) OK\n");
                }
                else if (frame.kind == ERROR){
                    printf("Received (src= 0) (ERROR)\n");
                }              
            }
            else if(cmd == "gtime"){
                msg = compGtime();
                int len = send_frame(sfd,GTIME,&msg);
                printf("Transmitted (src= %d) (GTIME)\n",id);
                int L = rcv(sfd,&frame);
                if (frame.kind == TIME){
                    printf("Received (src= 0) (TIME: )\n");
                }
                else if (frame.kind == ERROR){
                    printf("Received (src= 0) (ERROR)\n");
                }                
            }
            else if(cmd == "delay"){
                iss >> delay;
                printf("*** Entering a delay period of %d msec\n",delay);
                usleep(1000*delay);
                printf("*** Exiting delay period\n");
            }
            else if(cmd == "quit"){
                close(sfd);
                exit(0);
            }
            else{
                printf("Invalid command in the input file\n");
            }
        }
    }
    return 0;
}