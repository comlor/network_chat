#include <iostream>
#include <iomanip>
#include <vector>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <cassert>

#define PORT "30001"

struct sockaddr name;

void set_nonblock(int socket) {
    int flags;
    flags = fcntl(socket,F_GETFL,0);
    assert(flags != -1);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int agrc, char** argv) {
    int status, sock, adrlen;

    struct addrinfo hints;
    struct addrinfo *servinfo;  //will point to the results

    fd_set read_flags,write_flags; // the flag sets to be used
    struct timeval waitd;          // the max wait time for an event
    int sel;                      // holds return value for select();

    memset(&hints, 0, sizeof hints); //make sure the struct is empty
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; //tcp
    hints.ai_flags = AI_PASSIVE;     //use local-host address

    //get server info, put into servinfo
    if ((status = getaddrinfo("192.168.1.42", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    //make socket
    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (sock < 0) {
        printf("\nserver socket failure %m", sock);
        exit(1);
    }

    if(connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0) {
        printf("\nclient connection failure %m", sock);
        exit(1);
    }

    std::cout<<"\nSuccessful connection!\n";
    //sleep(10);
    //set_nonblock(sock);

    char* out = new char[255];
    char in[1024];// = new char[255];
    int numRead;
    int numSent;

    //int status;

    while(1)
    {
        try
        {
            memset(&in, 0, sizeof(in));
            status = recv(sock, in, sizeof(in), 0);
            std::cout<<"status: " << status << std::endl;
            std::cout<<"in: " << in << std::endl;
            if(status < 0)
            {
                fprintf(stderr, "RECV ERROR: %S\n", gai_strerror(status));
            }
            std::cout<<"TEST"<<std::endl;
            std::cout<<"RECV DATA: ";
            std::cout<<in<<std::endl;
            std::cout<<"AFTER"<<std::endl;
            break;
        }
        catch(...)
        {
            std::cout<<"RECV ERROR"<<std::endl;
        }
    }


    //while(1) {

        //waitd.tv_sec = 10;
        //FD_ZERO(&read_flags);
        //FD_ZERO(&write_flags);
        //FD_SET(sock, &read_flags);

        //if(strlen(out) != 0)
        //    FD_SET(sock, &write_flags);


        //sel = select(sock+1, &read_flags, &write_flags, (fd_set*)0, &waitd);
        //if(sel < 0)
        //    continue;



//        try
//        {
//            memset(&out, 0, sizeof(out));
//            out = "hello server!";
//            status = send(sock, out, strlen(out), 0);
//            fprintf(stderr, "SEND ERROR: %S\n", gai_strerror(status));
//            std::cout<<"\n SEND DATA: " << out << std::endl;
//            std::cout<<std::endl;

//        }
//        catch(...)
//        {
//            std::cout<<"RECV ERROR"<<std::endl;
//        }


        //close(sock);
        //socket ready for writing
//        if(FD_ISSET(sock, &write_flags))
//        {
//            std::cout<<"TESTING SEND"<<std::endl;


//            FD_CLR(sock, &write_flags);

//            memset(&out, 0, sizeof(out));
//        }
    //}   //end while

    std::cout<<"\n\nExiting normally\n";
    return 0;
}
