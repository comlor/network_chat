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

// ****************GLOBAL VARIABLES****************
struct tdata
{
    int thread_id;
    int status;
    int server;
    int time = 10;
    bool connection_waiting = false;
    bool exit_prog = false;
    std::vector<int> client_list;
    std::vector<struct sockaddr> client_addr;
};

void *get_in_addr(struct sockaddr *sa)
{
    if(sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

tdata list;
#define PORT = "30000"
#define CLIENT_IP = "192.168.1.42"
int NUM_ARGS = 5;

// ****************SERVER FUNCTIONS****************
void my_shell(int td);
int start_Server(std::string portNum);
int server_Accept(int server_sock, std::vector<int> &client_list);
int server_Send(int dest, int server, char *msg);
char *server_Recv(int server);
void server_Listener(int td);

// ****************TERMINAL FUNCTIONS****************
std::string read_input(void);
std::vector<std::string> parse_input(std::string &in);
int ex_args(std::vector<std::string> args);
void help();
void list_connections();

// ******************MAIN FUNCTION*****************
int main(int argc, char *argv[])
{
    std::cout<<std::endl<<std::endl;
    list.thread_id = 1;
    std::string port = "30001";
    int istatus = 1;

    int server = start_Server(port);
    list.server = server;

    std::thread terminal(my_shell,istatus);

    std::thread server_listen(server_Listener, istatus);
    server_listen.detach();

    terminal.join();
    list.connection_waiting = false;
    list.exit_prog = true;

    return 0;
}

// ******************THREAD FUNCTION*****************
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
            list.client_list.push_back(client_sock);
            list.connection_waiting = false;
        }
        else if(list.exit_prog == true)
            break;
    }
}

// *****************************Read Console Input****************************
std::string read_input(void)
{
    //std::cout<<"READ"<<std::endl;
    std::string in;
    std::getline(std::cin, in);
    std::fflush(NULL);
    return in;
}

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
    {
        help(); return 1;
    }
    else if(num_param == 1 && args[0] == "LIST" )
    {
        list_connections();
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
    sockaddr_in peeraddr;
    socklen_t size = sizeof(peeraddr);
    in_addr inaddr;

    char saddr[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &inaddr, saddr, INET_ADDRSTRLEN);

    std::cout << "inaddr: " << inaddr.s_addr << std::endl;
    std::cout << "saddr: " << saddr << std::endl;

    for(int i = 0; i < list.client_list.size(); i++)
        std::cout<<"host: " << list.client_list.at(i) << std::endl;
}

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
    list.client_list.push_back(client_sock);

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

