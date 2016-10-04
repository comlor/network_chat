#include <iostream>
#include <iomanip>
#include <vector>
#include <stdio.h>
#include <pthread.h>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>

//using namespace std;

// ****************************Global Variables****************************
int NUM_ARGS = 5;
#define NUM_THREADS 2

struct tdata        // Thread Data for server listener
{
    int thread_id;
    std::string input;
    std::vector<std::string> the_args;
    std::vector<int> client_list;
    int status = 1;
    int server;
    int time = 10;
};

//void error(char *msg)
//{
//    perror(msg);
//    exit(1);
//}

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

// ************************************************************************

// **************************Function Prototypes***************************
// Core Functions
void main_loop(std::string portNum);
std::string read_input(void);
std::vector<std::string> parse_input(std::string &);
int ex_args(std::vector<std::string>);
void *terminal_control(void* tdi);

// Parameter Functions
void help(); // Done
void myIP();
void myPort();
void connect(std::string dest, int port);
void listConnection();
void terminate(int connect_id);
void send(int connect_id, std::string message);
void exit(); // Done

// Server Functions
int start_Server(std::string portNum); //Returns File Descriptor of server
int server_Accept(int server_sock, std::vector<int> &client_list);
int server_Send(int dest, int server, char *msg);
void *server_Listener(void* td);        // Thread Listener Function

// ************************************************************************

// ******************************Main Function********************************
int main(int argc, char *argv[])
{
    std::cout<<std::endl<<std::endl<<std::endl;
    std::string portNum = "30001";

    main_loop(portNum);

    std::cout<<"EXIT SUCCESS"<<std::endl;
    return EXIT_SUCCESS;
}
// ***************************************************************************

// ****************************Main Program Loop******************************
void main_loop(std::__1::string portNum)
{
    pthread_t threads[NUM_THREADS];
    tdata td;

    std::string input;                  // For reading input from terminal
    std::vector<std::string> the_args;  // For storing list of args at terminal
    int status = 1;                     // General status for error messages
    td.client_list.resize(10);             // Sets list to handle 10 clients
    td.client_list.clear();                // Empties client list to start program

    int server = start_Server(portNum); // Starts up the TCP server and returns FD
    td.client_list.push_back(server);      // Adds server socket to client list[0]
    td.server = server;

    status = listen(td.server, 10);        // Listens for incoming connections
    status = server_Accept(td.server, td.client_list);// Accepts incoming connections

    int rcl = 0, rci = 0;

    while(status)
    {
        std::cout<<"> ";
        std::fflush(NULL);
        input = read_input();
        the_args = parse_input(input);
        status = ex_args(the_args);

//        td[0].thread_id = 0;
//        td[0].connect_waiting = false;
//        rcl = pthread_create(&threads[0], NULL,server_Listener, (void *)&td[0]);
//        if (rcl)
//        {
//           std::cout << "Error:unable to create thread," << rcl << std::endl;
//           exit(-1);
//        }

//        tdi.thread_id = 0;
//        rci = pthread_create(&threads[1], NULL,terminal_control, (void *)&tdi);
//        if (rci)
//        {
//           std::cout << "Error:unable to create thread," << rci << std::endl;
//           exit(-1);
//        }

//        if(td[0].connect_waiting == true)
//        {
//            status = server_Accept(server, client_list);
//            td[0].connect_waiting = false;
//        }

//        if(tdi.status == 0)
//        {
//            std::cout<<"EXITING"<<std::endl;
//            pthread_cancel(threads[0]);
//            status = 0;
//        }
    }
//    pthread_exit(threads[0]);
//    pthread_exit(threads[1]);

    for(unsigned int i = 0; i < td.client_list.size(); i++)
        shutdown(td.client_list.at(i), SHUT_RDWR);
}
// ***************************************************************************

void *terminal_control(void* tdi)
{
    struct tdata_input *data;
    data = (struct tdata_input *) tdi;
    std::string input = "";                  // For reading input from terminal
    std::vector<std::string> the_args;  // For storing list of args at terminal
    the_args.resize(2);
    the_args.clear();
    int tstatus = 1;                     // General status for error messages

    while(tstatus)
    {
        std::cout<<"> ";
        std::fflush(NULL);
        input = read_input();
        the_args = parse_input(input);
        the_args.push_back(input);
        tstatus = ex_args(the_args);
    }
    return(NULL);
}

// *****************************Read Console Input****************************
std::string read_input(void)
{
    //std::cout<<"READ"<<std::endl;
    std::string in;
    std::getline(std::cin, in);
    return in;
}
// ***************************************************************************

// *********************Parse Parameters from Console Input*******************
std::vector<std::string> parse_input(std::string &in)
{
    //std::cout<<"PARSE"<<std::endl;
    int num_param = 1;

    for(unsigned int i = 0; i < in.length(); i++)
    {
        if(in[i] == ' ')
            num_param++;
    }

    NUM_ARGS = num_param;
    std::vector<std::string> param;
    param.resize(num_param);
    int end = 0, index = 0;

    for(unsigned int i = 0; i < in.length(); i++)
    {
        end = in.find_first_of(' ', i);

        if(end == -1)
        {
            std::string insert = in.substr(i);
            param[index] = insert;
            for(unsigned int j = 0; j < param[index].length(); j++)
                param[index][j] = toupper(param[index][j]);
            break;
        }
        else
        {
            std::string insert = in.substr(i, end - i);
            param[index] = insert;
            for(unsigned int j = 0; j < param[index].length(); j++)
                param[index][j] = toupper(param[index][j]);
        }

        i = end;
        index++;
    }
    return param;
}
// ***************************************************************************

// ****************************Execute Parameters*****************************
int ex_args(std::vector<std::string> args)
{
    //std::cout<<"EXARG"<<std::endl;
    int num_param = NUM_ARGS;

    for(int i = 0; i < num_param; i++)
        std::cout<<"param check: " << args[i] << std::endl;

    if(num_param == 1 && args[0] == "EXIT" )
        return 0;
    else if(num_param == 1 && args[0] == "HELP" )
        help(); return 1;
}
// ***************************************************************************

void help()
{
    std::cout<<"****************************************************************** "<<std::endl;
    std::cout<<"-------------------------Chat Application------------------------- "<<std::endl;
    std::cout<<"****************************************************************** "<<std::endl;
    std::cout<<std::endl;
    std::cout<<"Chat is a communication application to communicate with other      "<<std::endl;
    std::cout<<"users and view network connection data using TCP connection        "<<std::endl;
    std::cout<<std::endl                                                                 <<std::endl;
    std::cout<<"Command List:                                                      "<<std::endl;
    std::cout<<"help - displays this help document"                           <<std::endl<<std::endl;
    std::cout<<"myip - displays your network ip address"                      <<std::endl<<std::endl;
    std::cout<<"myport - displays the port number used "                      <<std::endl<<std::endl;
    std::cout<<"connect <destination> <port no> - creates new connection to"        <<std::endl;
    std::cout<<"      specified ip and port"                                  <<std::endl<<std::endl;
    std::cout<<"list - lists all the open connections this process is part of"<<std::endl<<std::endl;
    std::cout<<"terminate <connection id> - terminates the connection of the       "<<std::endl;
    std::cout<<"      specified id number                                    "<<std::endl<<std::endl;
    std::cout<<"send <connection id> <message> - sends a message to the            "<<std::endl;
    std::cout<<"      specified connection id.                               "<<std::endl<<std::endl;
    std::cout<<"exit - Close all connection and terminate program            "<<std::endl<<std::endl;
    std::cout<<"****************************************************************** "<<std::endl;
}

// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************
// ***************************************************************************************************

int start_Server(std::string portNum)
{
    struct addrinfo hints,      //Hints - addrinfo struct to configure networking
                    *servinfo,  //ServInfo - Struct holds info returned from getaddrinfo()
                    *p;         //Walker pointer to iterate through servinfo struct
    int server_sock,            //Server_Sock - File descriptor that will be returned
            ret_code;           //ret_Code - hold return status from getaddrinfo()


    memset(&hints, 0, sizeof hints);//Clear memory space for hints
    hints.ai_family = AF_INET;      //Sets IPV4
    hints.ai_socktype = SOCK_STREAM;//Sets TCP
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Use getaddrinfo() to build networking information for server, stored in servinfo
    ret_code = getaddrinfo(NULL, portNum.c_str(), &hints, &servinfo);
    if (ret_code != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret_code));
        exit(EXIT_FAILURE);
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        // Create server socket and store file descriptor in server_sock
        server_sock = socket(p->ai_family, p->ai_socktype,p->ai_protocol);

        if (server_sock == -1)
            continue;


        if (bind(server_sock, p->ai_addr, p->ai_addrlen) == 0)
        {
            break;                  /* Success */
        }

        close(server_sock);
    }

    if (p == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(servinfo);           /* No longer needed */

    return server_sock;
}

int server_Accept(int server_sock, std::vector<int> &client_list)
{
    struct sockaddr_storage client;
    int client_sock, status;
    socklen_t len = sizeof(client);

    client_sock = accept(server_sock, (struct sockaddr*)&client, &len);
    client_list.push_back(client_sock);

    char *out;

    memset(&out, 0, sizeof(out));
    out = "This is a chat app!";
    status = send(client_sock, &*out, strlen(out), 0);
    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

    close(client_sock);

    return 1;
}

int server_Send(int dest, int server, char *msg)
{
    int status = send(dest, &*msg, strlen(msg), 0);

    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

    return status;
}

char* server_Recv(int server)
{
    char *in = new char[255];
    int status;
    memset(&in, 0, sizeof(in));
    status = recv(server, in, sizeof(in), 0);

    if(status < 0)
        fprintf(stderr, "RECV ERROR: %s\n", gai_strerror(status));
    return in;
}

void *server_Listener(void* td)
{
    struct tdata *data;

    data = (struct tdata *) td;
    data->status = listen(data->server, data->time);
    //std::cout<<"status: " << data->status << std::endl;
    //if(data->status < 0)
    //    fprintf(stderr, "RECV ERROR: %s\n", gai_strerror(data->status));
    return(NULL);
}




























/*
 *     // We should wait now for a connection to accept
    int client_sock, status;
    struct sockaddr_storage client_addr;
    socklen_t addr_size;
    char s[INET6_ADDRSTRLEN]; // an empty string

    // Calculate the size of the data structure
    addr_size = sizeof client_addr;

    printf("I am now accepting connections ...\n");

    while(1)
    {
        // Accept a new connection and return back the socket desciptor
        client_sock = accept(server_sock, (struct sockaddr *) & client_addr, &addr_size);
        if(client_sock < 0)
        {
            fprintf(stderr,"accept: %s\n",gai_strerror(client_sock));
            continue;
        }

        inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr *) &client_addr),s ,sizeof s);
        printf("I am now connected to %s \n",s);


        char *out = new char[255];
        try
        {
            std::cout<<"SENDING"<<std::endl;
            memset(&out, 0, sizeof(out));
            out = "HELLO CLIENT!";
            status = send(client_sock, out, strlen(out), 0);
            fprintf(stderr,"SEND ERROR: %s\n",gai_strerror(status));
            std::cout<<std::endl;
        }
        catch(...)
        {
            std::cout<<"SEND ERROR: " << status <<std::endl;
        }

//        char *in = new char[255];
//        try
//        {
//            std::cout<<"RECEIVING"<<std::endl;
//            memset(&in, 0, sizeof(in));
//            status = recv(client_sock, in, sizeof(in), 0);
//            fprintf(stderr,"RECV ERROR: %s\n",gai_strerror(status));
//            std::cout<<std::endl;
//            std::cout<<"in: " << in << std::endl;
//        }
//        catch(...)
//        {
//            std::cout<<"RECV ERROR: " << status <<std::endl;
//        }

        //if(status == -1)
        //{
        //    close(client_sock);
        //    _exit(4);
        //}

    }
*/

