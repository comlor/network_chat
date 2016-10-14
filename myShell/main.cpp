#include <iostream>
#include <vector>
#include <thread>

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <signal.h>
#include <ifaddrs.h>
#include <net/if.h>

enum MY_ERRORS { CONNECT_FAIL, SEND_FAIL, RECV_FAIL, BIND_FAIL, CLOSE_FAIL, ARG_ERROR };

// ****************GLOBAL VARIABLES****************
struct tdata //Needs cleaned up.  Some variables no longer needed.
{
    int thread_id;
    int status;
    int server;
    std::string listen_port;
    int time = 10;
    bool connection_waiting = false;
    bool exit_prog = false;
    std::vector<int> client_list;
    std::vector<std::string> client_list_ad;
    std::vector<std::string> client_list_port;
    std::vector<sockaddr_in> client_addr;
    sockaddr_storage server_sock;
    struct addrinfo *serv;
};

void *get_in_addr(struct sockaddr *sa)//Copy and paste code from internet.  Will grab link to cite source
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

tdata list;
int NUM_ARGS = 5;

// ****************SERVER FUNCTIONS****************
int start_Server(std::string portNum);
int server_Accept(int server_sock);

// ****************TERMINAL FUNCTIONS****************
std::string read_input(void);
std::vector<std::string> parse_input(std::string &in);
int ex_args(std::vector<std::__1::string> &args);
void help();
void list_connections();
void get_myip();
void connect_client(std::__1::string con_ip, std::__1::string con_port);
void terminate_socket(int sock);
void send_msg(int sock, std::__1::string msg);
void error_msg(int err_num);

// ****************THREAD FUNCTIONS****************
void my_shell(int td);
void server_Recv(int server);
void server_Listener(int td);

// ******************MAIN FUNCTION*****************
int main(int argc, char *argv[])//Need to update to take port as argument, currently hardcoded.
{
    std::cout<<std::endl<<std::endl;
    std::string port;
    list.thread_id = 1;
    list.client_list.resize(10);
    list.client_list.clear();
    list.listen_port = port;
    int istatus = 1;

//    if(argc != 2)
//        error_msg(ARG_ERROR);
//    else
//        port = argv[1];
    port = "5001";

    int server = start_Server(port);
    list.server = server;

    std::thread terminal(my_shell,istatus);

    std::thread server_listen(server_Listener, istatus);
    server_listen.detach();

    terminal.join();

    list.connection_waiting = false;
    list.exit_prog = true;

    //shutdown(list.server, 2);

    return 0;
}

// ******************THREAD FUNCTIONs*****************
void my_shell(int td)
{
    int status = 1;
    std::string input;
    std::vector<std::string> the_args;

    while(status)
    {
        std::cout<<"> ";
        input = read_input();
        the_args = parse_input(input);
        status = ex_args(the_args);
    }

    //for(int i = 0; i < list.client_list.size(); i++)//Should write shutdown function to close all connections
    //    shutdown(list.client_list[i], 2);
}

void server_Listener(int td)
{
    int client_sock, status;

    while(list.exit_prog == false)
    {
        status = listen(list.server, list.time);

        if(status == 0)
            list.connection_waiting = true;

        if(list.connection_waiting == true)
        {
            client_sock = server_Accept(list.server);
            list.connection_waiting = false;
        }
        else if(list.exit_prog == true)
            break;
    }
}

void server_Recv(int server)
{
    char in[1024];
    size_t buff_size = 1024;
    int status = 0;

    while(1)
    {
        memset(&in, 0, sizeof(in));
        status = recv(server, in, buff_size, MSG_DONTWAIT);
        if(status < 0)
        {
            fprintf(stderr, "RECV ERROR: %S\n", gai_strerror(status));
        }
        else if(status > 0)
            std::cout<<"data: " << in << std::endl;
        else if(status == 0)
            break;
    }

    //std::cout<<"data: " << in << std::endl;
    //std::cout<<"status: " << status << std::endl;
}

// ***************************************************************************
// *****************************Read Console Input****************************
std::string read_input(void)
{
    std::string in;
    std::getline(std::cin, in);
    std::fflush(NULL);
    return in;
}

// *********************Parse Parameters from Console Input*******************
std::vector<std::string> parse_input(std::string &in)
{
    int num_param = 1;

    for(unsigned int i = 0; i < in.length(); i++)
    {
        if(in[i] == ' ')
            num_param++;
    }

    NUM_ARGS = num_param;
    std::vector<std::string> param;
    param.resize(num_param);
    param.clear();
    int end = 0, index = 0;

    for(unsigned int i = 0; i < in.length(); i++)
    {
        end = in.find_first_of(' ', i);

        if(end == -1)
        {
            std::string insert = in.substr(i);
            param.push_back(insert);
            break;
        }
        else
        {
            std::string insert = in.substr(i, end - i);
            param.push_back(insert);
        }
        index++;
        i = end;
    }

    int first = 0;
    for(unsigned int j = 0; j < param[first].length(); j++)
        param[first][j] = toupper(param[first][j]);

    return param;
}

// ****************************Execute Parameters*****************************
int ex_args(std::vector<std::string> &args)
{
    int num_param = NUM_ARGS;

    if(num_param == 1 && args[0] == "EXIT" )
        return 0;
    else if(num_param == 1 && args[0] == "HELP" )
    {
        help(); return 1;
    }
    else if(num_param == 1 && args[0] == "LIST" )
    {
        list_connections();
        return 1;
    }
    else if(num_param == 1 && args[0] == "MYIP")
    {
        get_myip();
        return 1;
    }
    else if(num_param == 1 && args[0] == "MYPORT")
    {
        std::cout<<"> Listening Port: " << list.listen_port << std::endl;
        return 1;
    }
    else if(num_param > 1 && args[0] == "TERMINATE")
    {
        terminate_socket(std::stoi(args[1]));
        return 1;
    }
    else if(num_param > 1 && args[0] == "CONNECT")
    {
        connect_client(args[1], args[2]);
        return 1;
    }
    else if(num_param > 1 && args[0] == "SEND")
    {
        std::string msg;
        msg.clear();
        if(args.size() > 2)
        {
            for(int i = 2; i < args.size(); i++)
            {
                msg += args[i];
                msg += " ";
            }
        }
        send_msg(std::stoi(args[1]), msg);
        return 1;
    }
}

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

void list_connections()
{
    std::cout<<"ID\tIP\t\tPORT"<<std::endl;

    for(int i = 0; i < list.client_list.size(); i++)
    {
        std::cout<<list.client_list[i]<<"\t";
        std::cout<<list.client_list_ad[i]<<"\t";
        std::cout<<list.client_list_port[i]<<std::endl;
    }
}

void get_myip()//This is copy and paste code from net.  Shows ip's for all interfaces on computer
{
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;
    char buf[64];

    if(getifaddrs(&myaddrs) != 0)
    {
        std::cout<<"MYIP ERROR"<<std::endl;
        exit(1);
    }

    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)
            continue;
        if (!(ifa->ifa_flags & IFF_UP))
            continue;

        switch (ifa->ifa_addr->sa_family)
        {
            case AF_INET:
            {
                struct sockaddr_in *s4 = (struct sockaddr_in *)ifa->ifa_addr;
                in_addr = &s4->sin_addr;
                break;
            }

            case AF_INET6:
            {
                struct sockaddr_in6 *s6 = (struct sockaddr_in6 *)ifa->ifa_addr;
                in_addr = &s6->sin6_addr;
                break;
            }

            default:
                continue;
        }

        if (!inet_ntop(ifa->ifa_addr->sa_family, in_addr, buf, sizeof(buf)))
        {
            printf("%s: inet_ntop failed!\n", ifa->ifa_name);
        }
        else
        {
            printf("%s: %s\n", ifa->ifa_name, buf);
        }
    }

    //freeifaddrs(myaddrs);
}

void connect_client(std::string con_ip, std::string con_port)
{
    int sockfd = 0;
    struct sockaddr_in client_info;

    client_info.sin_family = AF_INET;
    client_info.sin_port = htons(5001);

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Error : Could not create socket \n");
        exit(1);
    }

    if(inet_pton(AF_INET, con_ip.c_str(), &client_info.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        exit(1);
    }

    if( connect(sockfd, (struct sockaddr *)&client_info, sizeof(client_info)) < 0)
    {
       printf("\n Error : Connect Failed \n");
       exit(1);
    }

    std::cout<<"\nSuccessful Connection!\n";

    //Add Client to list of connected TCP Sockets
    list.client_list.push_back(sockfd);
    list.client_list_ad.push_back(con_ip);
    list.client_list_port.push_back(con_port);

    std::thread receive_func(server_Recv, sockfd);

    while(!receive_func.joinable()){}
    receive_func.detach();
}

void terminate_socket(int sock)
{
    //shutdown(sock,SHUT_RDWR);

    for(int i = 0; i < list.client_list.size(); i++)
    {
        if(list.client_list[i] == sock)
        {
            list.client_list[i] = -1;
            list.client_list_ad[i] = "";
            list.client_list_port[i] = "";
        }
    }

    //shutdown(sock,2);
}

void send_msg(int sock, std::string msg)
{
    int status;
    const char *out = msg.c_str();

    status = send(sock, msg.c_str(), strlen(out), 0);
    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }
}


// ***************************************************************************
// ******************************Server Functions*****************************

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

        //close(server_sock);
    }

    if (p == NULL) {               /* No address succeeded */
        fprintf(stderr, "Could not bind\n");
        exit(EXIT_FAILURE);
    }

    list.server = server_sock;
    list.serv = servinfo;
    //freeaddrinfo(servinfo);           /* No longer needed */

    return server_sock;
}

int server_Accept(int server_sock)
{
    struct sockaddr_in client;
    int client_sock, status;
    char ip_string[INET6_ADDRSTRLEN];
    socklen_t len = sizeof(client);

    client_sock = accept(server_sock, (struct sockaddr *) &client, &len);

    inet_ntop(AF_INET, (struct sockaddr_in *)&client.sin_addr, ip_string, sizeof ip_string);

    if (getsockname(client_sock, (struct sockaddr *)&client, &len) == -1)
        std::cout<<"PORT NUMBER ERROR"<<std::endl;

    std::string port = std::to_string(ntohs(client.sin_port));

    list.client_list.push_back(client_sock);
    list.client_list_ad.push_back(ip_string);
    list.client_list_port.push_back(port);
    std::cout<<"port: " << port << std::endl;
    std::cout<<"ip: " << ip_string << std::endl;

    char *out;

    memset(&out, 0, sizeof(out));
    out = "Welcome to the Chat!";
    status = send(client_sock, &*out, strlen(out), 0);
    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

    std::thread receive_func(server_Recv, client_sock);

    while(!receive_func.joinable()){}
    receive_func.detach();

    return 1;
}

void error_msg(int err_num)
{
    switch(err_num)
    {
    case ARG_ERROR:
        std::cout<<"Invalid Parameters: chat <port>"<<std::endl;
        break;
    default:
        std::cout<<"Unknown Error Occured"<<std::endl;
        break;
    }
}

