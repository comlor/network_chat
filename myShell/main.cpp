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
    std::vector<uint16_t> client_list_port;
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
int server_Accept(int server_sock, std::vector<int> &client_list);

// ****************TERMINAL FUNCTIONS****************
std::string read_input(void);
std::vector<std::string> parse_input(std::string &in);
int ex_args(std::vector<std::string> args);
void help();
void list_connections();
void get_myip();
void connect_client(std::__1::string con_ip, std::__1::string con_port);
void terminate_socket(int sock);
void send_msg(int sock, std::__1::string msg);

// ****************THREAD FUNCTIONS****************
void my_shell(int td);
void server_Recv(int server);
void server_Listener(int td);

// ******************MAIN FUNCTION*****************
int main(int argc, char *argv[])//Need to update to take port as argument, currently hardcoded.
{
    std::cout<<std::endl<<std::endl;
    list.thread_id = 1;
    list.client_list.resize(10);
    list.client_list.clear();
    //list.client_list_ad.resize(10);
    //list.client_list_ad.clear();
    std::string port = "5001";
    list.listen_port = port;
    int istatus = 1;

    int server = start_Server(port);
    list.server = server;

    std::thread terminal(my_shell,istatus);

    std::thread server_listen(server_Listener, istatus);
    server_listen.detach();

    terminal.join();
    list.connection_waiting = false;
    list.exit_prog = true;

    shutdown(list.server, 2);

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
        the_args.push_back(input);
        status = ex_args(the_args);
    }

    for(int i = 0; i < list.client_list.size(); i++)//Should write shutdown function to close all connections
        shutdown(list.client_list[i], 2);
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
            client_sock = server_Accept(list.server,list.client_list);
            list.connection_waiting = false;
        }
        else if(list.exit_prog == true)
            break;
    }
}

void server_Recv(int server)
{
    std::cout<<"RECEVING"<<std::endl;
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

// ****************************Execute Parameters*****************************
int ex_args(std::vector<std::string> args)
{
    int num_param = NUM_ARGS;

    for(int i = 0; i < num_param; i++)
        std::cout<<"param check: " << args[i] << std::endl;

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
        std::string msg = args[2];
        if(args.size() > 3)
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
//    socklen_t len;
//    struct sockaddr_storage addr;
//    char ipstr[INET6_ADDRSTRLEN];
//    int port;

//    len = sizeof addr;

//    for(int i = 0; i < list.client_addr.size(); i++)
//    {
//        struct sockaddr_in *s = (struct sockaddr_in *)&list.client_addr.at(i);
//        port = ntohs(s->sin_port);
//        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
//        std::cout<<"host: " << list.client_list.at(i);
//        std::cout<<" - "<< ipstr <<std::endl;
//        ipstr[0] = '\0';
//    }

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

    freeifaddrs(myaddrs);
}

void connect_client(std::string con_ip, std::string con_port)
{
    int sock, status;
    struct addrinfo *servinfo, tcp_info;

    const char *conip = con_ip.c_str();
    const char *conport = con_port.c_str();

    memset(&tcp_info, 0, sizeof(tcp_info));
    tcp_info.ai_family = AF_INET;
    tcp_info.ai_socktype = SOCK_STREAM;
    tcp_info.ai_flags = AI_PASSIVE;

    if((status = getaddrinfo(conip, conport, &tcp_info, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    sock = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sock < 0)
    {
        printf("\nServer socket failure %m", sock);
        exit(1);
    }

    if(connect(sock, servinfo->ai_addr, servinfo->ai_addrlen) < 0)
    {
        printf("\nClient connection failure %m", sock);
        exit(1);
    }
    uint16_t p = std::stoi(conport);
    std::cout<<"\nSuccessful Connection!\n";
    list.client_list.push_back(sock);
    list.client_list_ad.push_back(conip);
    list.client_list_port.push_back(p);

    std::thread receive_func(server_Recv, sock);

    while(!receive_func.joinable()){}
    receive_func.detach();
}

void terminate_socket(int sock)
{
    shutdown(sock,SHUT_RDWR);

    for(int i = 0; i < list.client_list.size(); i++)
    {
        if(list.client_list[i] == sock)
        {
            list.client_list[i] = -1;
            list.client_list_ad[i] = "";
            list.client_list_port[i] = 0;
        }
    }

    shutdown(sock,2);
}

void send_msg(int sock, std::string msg)
{
    int client_sock, status;

    char *out = (char*)malloc(255 * sizeof(char));
    memset(&out, 0, sizeof(out));
    out = "Hello!";

    status = send(sock, msg.c_str(), strlen(out), 0);
    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

//    int status = send(sock, &msg, std::strlen(msg.c_str()), 0);

//    if(status < 0)
//    {
//        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
//        std::cout<<std::endl;
//    }
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

    list.serv = servinfo;
    //freeaddrinfo(servinfo);           /* No longer needed */

    return server_sock;
}

int server_Accept(int server_sock, std::vector<int> &client_list)
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

    connect_client(ip_string, port);

    char *out;

    memset(&out, 0, sizeof(out));
    out = "This is a chat app!";
    status = send(client_sock, &*out, strlen(out), 0);
    if(status < 0)
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

    return 1;
}

