// 1. Resources used to create this program
// 2. http://code.runnable.com/VXjZZimG7Nk0smWF/simple-tcp-server-code-for-c%2B%2B-and-socket
// http://man7.org/linux/man-pages/man3/getifaddrs.3.html
// http://man7.org/linux/man-pages/man3/inet_ntop.3.html
// http://man7.org/linux/man-pages/man2/socket.2.html
// http://man7.org/linux/man-pages/man3/inet_pton.3.html
// http://man7.org/linux/man-pages/man2/connect.2.html
// http://man7.org/linux/man-pages/man2/shutdown.2.html
// http://man7.org/linux/man-pages/man2/send.2.html
// http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
// http://man7.org/linux/man-pages/man2/accept.2.html

#include <iostream>
#include <fstream>
#include <vector>
#include <climits>
#include <thread>
#include <chrono>
#include <cstring>
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

enum MY_ERRORS { CONNECT_FAIL, SEND_FAIL, RECV_FAIL, BIND_FAIL, CLOSE_FAIL };

// ****************GLOBAL VARIABLES****************
struct server_info // Neighbor server informations neighbor id corresponds to serv_id in neighbor_info
{
    int server_id;
    std::string server_ipaddr;
    int server_port;
};

struct neighbor_info // A record for the routing table
{
    int serv_id;
    int neighbor_id;
    int link_cost;
};

struct route_table
{
    bool send_update = false; //Time to send an update
    int update_interval = 60; //Sets default update interval to 30 seconds -> 30,000 milliseconds
    int num_servers; // Number of servers in table
    int num_neighbors; // Number of neighbors this server has
    std::vector<server_info> server_list; // list of server ids, ipaddresses, and port numbers
    std::vector<neighbor_info> neighbor_list; // Routing table of server - neightbor - link cost
};
route_table my_table;

struct tdata //Needs cleaned up.  Some variables no longer needed.
{
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

    int my_server_id;
    int packet_count = 0;
};
tdata list;
int NUM_ARGS = 5;

// ****************SERVER FUNCTIONS****************
int start_Server(std::string portNum);
int server_Accept(int server_sock, std::vector<int> &client_list);

// ****************ROUTE FUNCTIONS*******************
void readTable(std::string file_name, route_table &myself);
void connectNeighbors();
void updateCost(int server_id, int neighbor_id, int new_cost);
void increasePacketCount();
void resetPacketCount();
void disable(int server_id);
void crash();
int getServerInfo();
int getServerbyIP(std::string ip);
bool isNeighbor(int id);
server_info getNeighborInfo(int neighbor_id);
std::string create_packet(int num_fields, int serv_port, std::string serv_ip, route_table the_routes);

// ****************TERMINAL FUNCTIONS****************
std::string read_input(void);
std::vector<std::string> parse_input(std::string &in);
int ex_args(std::vector<std::string> &args);
void help();
void display();
std::string get_myip();
void connect_client(std::string con_ip, std::string con_port);
void terminate_socket(int sock);
void send_msg(int sock, std::string msg);
void list_connections();
void updateNeighbors();

// ****************THREAD FUNCTIONS****************
void my_shell(int status);
void server_Recv(int server);
void server_Listener(int status);
void sendSchedule(int time);

// ******************MAIN FUNCTION*****************
int main(int argc, char *argv[])//Need to update to take port as argument, currently hardcoded.
{
    std::cout<<std::endl<<std::endl;

    // Set up client list to support 10 clients without additional overhead
    // to resize list.
    list.client_list.resize(10);
    list.client_list.clear();
    std::string port = "5001";   // store port number
    int istatus = 1;    // status variable for mainloop

    my_table.neighbor_list.resize(my_table.num_neighbors);
    my_table.server_list.resize(my_table.num_servers);

    std::string file_name;

    if(argc < 5 || argc > 5)
        std::cout<<"argument error: program -t <route file> -i <update interval>"<<std::endl;
    else
    {
        if(strcmp(argv[1],"-t") == 0)
            file_name = argv[2];

        if(strcmp(argv[3], "-i") == 0)
            my_table.update_interval = std::stoi(argv[4]);

        std::cout<<argv[0]<<std::endl;
        std::cout<<argv[1]<<std::endl;
        std::cout<<argv[2]<<std::endl;
        std::cout<<argv[3]<<std::endl;
        std::cout<<argv[4]<<std::endl;
    }

    //Macbook Pro
    //std::string file_name = "/Users/chrisomlor/Documents/QT_Projects/dist_vect/dist_vec/routing.txt";
    //IMAC
    //std::string file_name = "/Users/comlor/Desktop/dist_vect/dist_vec/routing.txt";
    //DELL
    //std::string file_name = "/home/chrisomlor/Dropbox/dist_vect/dist_vec/routing.txt";
    //HP
    //std::string file_name = "/home/comlor/Desktop/dist_vect/dist_vec/routing.txt";

    readTable(file_name, my_table);

    std::string ip;

    int index = getServerInfo();
    server_info my_server = my_table.server_list[index];

    if(index == -1)
        std::cout<<"Unable to find server information in table"<<std::endl;
    else
    {
        list.my_server_id = my_server.server_id;
        ip = my_server.server_ipaddr;
        port = std::to_string(my_server.server_port);
    }

    std::cout<<"ip: " << ip << std::endl;
    std::cout<<"port: " << port << std::endl;

    // Determine if args from command line is valid
//    if(argc < 2)
//        std::cout<<"> Two few arguments - use --HELP switch for assitance"<<std::endl;
//    else
//    {
//        if(strcmp(argv[1],"HELP") == 0)
//            help();
//        else
//            port = argv[1];
//    }

    list.listen_port = port;    // Set listen port for the server
    int server = start_Server(port); // Starts up server and binds to ip, returns server socket
    list.server = server;   // Store Server Socket

    std::thread terminal(my_shell,istatus); // Starts thread for user interface

    std::thread server_listen(server_Listener, istatus); // Starts thread to listen for connections
    server_listen.detach(); // Detach listening thread so it will run on it's own

    sleep(10);
    connectNeighbors();

    sleep(5);
    std::thread auto_update(sendSchedule, my_table.update_interval);
    auto_update.detach();

    terminal.join(); // Wait for terminal thread to exit.  This happens when user types exit command
    list.connection_waiting = false; // After terminal thread exits, sets flags to force receive and
    list.exit_prog = true;           // listening thread to exit for clean shutdown.

    // On exit from program shutdown connections on all connected sockets.
    for(int i = 0; i < list.client_list.size(); i++)
        shutdown(list.client_list[i], SHUT_RDWR);

    // On exit shutdown the server socket.
    shutdown(list.server, SHUT_RDWR);

    return 0;
}

// ******************THREAD FUNCTIONs*****************
// **My_Shell -- Main User Interface Function
void my_shell(int status)
{
    std::string input; // Input from user commands
    std::vector<std::string> the_args; // Parsed list of input.

    while(status)
    {
        std::cout<<"> ";
        input = read_input();           // Read console input
        the_args = parse_input(input);  // Parse input from console
        status = ex_args(the_args);     // Execute the users command.
    }
}

// ** Server_Listener -- Listens for incoming connection requests
void server_Listener(int status)
{
    int client_sock; // Return value from Server_Accept function
    status = 0;

    while(list.exit_prog == false) // Runs infinitely until user types exit
    {                              // causing this flag to change and exit thread
        status = listen(list.server, list.time);// Listen function, blocking waiting for request

        if(status == 0) // Sets connection waiting flag to allow accept function to execute
            list.connection_waiting = true;

        if(list.connection_waiting == true)
        {
            client_sock = server_Accept(list.server,list.client_list);
            list.connection_waiting = false;
        }
        else if(list.exit_prog == true) // Exit thread if listen timeout occurs and no connection waiting
            break;
    }
}

// ** server_Recv -- Thread called with every connection to server to receive any packets
//                  server parameter is the socket to receive for a specific client.
void server_Recv(int server)
{
    char in[1024];          // Input buffer for incoming packet data
    size_t buff_size = 1024;
    int status = 0;         // Status hold error codes for socket functions

    while(1) // Run infinitely until status is zero indicating connection is closed
    {
        memset(&in, 0, sizeof(in)); // Clear data from memory before each receive operation

        // Receive packet on specific socket in nonblocking mode
        status = recv(server, in, buff_size, MSG_DONTWAIT);
        if(status < 0){} // Error occured, which is expected since nonblocking mode for recv
        else if(status > 0)// Data received
        {
            std::string msg = in;
            std::string command = msg.substr(0,msg.find_first_of(","));
            std::string the_msg = msg.substr(msg.find_first_of(","));

            if(strcmp(command.c_str(),"_TERMINATE_") == 0) // Received termination of socket flag
            {
                int i = 0;
                for(i = 0; i < list.client_list.size(); i++) // Find index of socket in list
                    if(list.client_list[i] == server)        // to display closed message.
                        break;
                std::cout<<"> Connection "<<i<<" closed by "<<list.client_list_ad[i]<<std::endl;
                list.client_list[i] = -1; // Erase connection from connection list.
                list.client_list_ad[i] = "";
                list.client_list_port[i] = "";

                the_msg = the_msg.substr(1);
                std::string neighbor = the_msg.substr(0, the_msg.find_first_of(","));
                std::string server = the_msg.substr(the_msg.find_first_of(",") + 1);
                std::cout<<"server: " << server << "  -  neighbor: " << neighbor << std::endl;
                updateCost(std::stoi(server),std::stoi(neighbor),INT_MAX);

                break;
            }
            else if(strcmp(command.c_str(),"_CRASH_") == 0)
            {
                std::string param = msg.substr(msg.find_first_of(","));
                int serv_id = std::atoi(param.c_str());
                updateCost(list.my_server_id,serv_id,INT_MAX);

                int i = 0;
                for(i = 0; i < list.client_list.size(); i++) // Find index of socket in list
                    if(list.client_list[i] == server)        // to display closed message.
                        break;
                std::cout<<"> Connection "<<i<<" closed by "<<list.client_list_ad[i]<<std::endl;
                list.client_list[i] = -1; // Erase connection from connection list.
                list.client_list_ad[i] = "";
                list.client_list_port[i] = "";
                break;
            }
            else if(strcmp(command.c_str(), "_MESSAGE_") == 0)
            {
                //Display message details from sending client.
                int i = 0;
                for(i = 0; i < list.client_list.size(); i++)
                    if(list.client_list[i] == server)
                        break;
                //std::cout<<"> Message received from "<< list.client_list_ad[i] << std::endl;
                //std::cout<<"> Sender's Port: " << list.client_list_port[i] << std::endl;
                //std::cout<<"> Message: " << the_msg << std::endl;
            }
            else
            {
                //std::cout<<"Test ReCV"<<std::endl;
                the_msg = the_msg.substr(1);
                std::vector<std::string> msg_list;
                msg_list.resize(10);
                msg_list.clear();
                int end = 0, index = 0;

                for(unsigned int i = 0; i < the_msg.length(); i++)
                {
                    end = the_msg.find_first_of(',', i);

                    if(end == -1)
                    {
                        std::string insert = the_msg.substr(i);
                        msg_list.push_back(insert);
                        break;
                    }
                    else
                    {
                        std::string insert = the_msg.substr(i, end - i);
                        msg_list.push_back(insert);
                    }
                    index++;
                    i = end;
                }

                //for(unsigned int i = 0; i < msg_list.size(); i++)
                //    std::cout<<i << ": " << msg_list[i] << std::endl;
                std::string serv_ip = msg_list[2];
                int recv_id;
                for(unsigned int i = 0; i < my_table.num_servers; i++)
                {
                    server_info the_server = my_table.server_list[i];
                    if(strcmp(serv_ip.c_str(),the_server.server_ipaddr.c_str()) == 0)
                        recv_id = the_server.server_id;
                }

                std::cout<<"Received a message from server " << recv_id << std::endl;

                for(unsigned int i = 4; i < msg_list.size(); i += 4)
                {
                    //std::cout<<"S1: " << msg_list[i-2] << ", S2: " << msg_list[i] << ", COST: " << msg_list[i+1] << std::endl;
                    updateCost(getServerbyIP(msg_list[i-2]),std::atoi(msg_list[i].c_str()),std::atoi(msg_list[i+1].c_str()));
                }
                list.packet_count += 1;
            }
        }
        else if(status == 0) // Exit receive thread when connection is close
            break;
    }
}

void sendSchedule(int time)
{
    while(!list.exit_prog)
    {
        std::this_thread::sleep_for(std::chrono::seconds(time));

        if(!list.exit_prog)
            updateNeighbors();
    }
}

// ***************************************************************************
// *****************************Client Functions******************************
// **Read Console Input -- returns the string from user
std::string read_input(void)
{
    std::string in;
    std::getline(std::cin, in);
    std::fflush(NULL);
    return in;
}

// **Parse Input from the console -- returns list of parsed strings
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

// **Execute the Parsed Arguments -- Call appropriate function based on command
int ex_args(std::vector<std::string> &args)
{
    int num_param = NUM_ARGS;

    if(num_param == 1 && args[0] == "EXIT" ) // exit the program
        return 0;
    else if(num_param == 1 && args[0] == "HELP" ) // show help for program
    {
        help(); return 1;
    }
    else if(num_param == 1 && args[0] == "LIST")
    {
        list_connections();
        return 1;
    }
    else if(num_param == 1 && args[0] == "DISPLAY" ) // Show list of all open connections
    {
        display();
        return 1;
    }
    else if(num_param == 1 && args[0] == "MYIP") // Show the binded IP Address of local computer
    {
        printf("> The IP address is: %s\n", get_myip().c_str());
        return 1;
    }
    else if(num_param == 1 && args[0] == "MYPORT") // Show the listening port of local computer
    {
        std::cout<<"> Listening Port: " << list.listen_port << std::endl;
        return 1;
    }
    else if(num_param == 1 && args[0] == "STEP")
    {
        updateNeighbors();
        std::cout<<args[0] << " SUCCESS" << std::endl;
        return 1;
    }
    else if(num_param == 1 && args[0] == "PACKETS")
    {
        std::cout<<"Routing Packets Received: " << list.packet_count << std::endl;
        resetPacketCount();
        std::cout<<args[0] << " SUCCESS" << std::endl;
        return 1;
    }
    else if(num_param == 1 && args[0] == "CRASH")
    {
        //Send crash message to neighbors
        crash();
        std::cout<<args[0] << " SUCCESS" << std::endl;
        return 1;
    }
    else if(num_param > 1 && args[0] == "DISABLE")
    {
        try
        {
            if(std::isdigit((int)args[1][0]))
            {
                if(isNeighbor(std::atoi(args[1].c_str())))
                {   //Need to get socket FD from the neighbor_id number
                    int sock = std::stoi(args[1]);
                    std::string term = "_TERMINATE_,";
                    term += std::to_string(list.my_server_id);
                    term += ",";
                    term += args[1][0];
                    std::cout<<"term: " << term << std::endl;
                    send_msg(sock,term);
                    updateCost(list.my_server_id,std::stoi(args[1]),INT_MAX);

                    terminate_socket(std::atoi(args[1].c_str()));
                    std::cout<<args[0] << " SUCCESS" << std::endl;
                }
                else
                {
                    std::cout<<"ERROR: Server is not a neighbor"<<std::endl;
                }
            }
        }
        catch(...)
        {
            std::cout<<"UNKNOWN ERROR OCCURED"<<std::endl;
        }

        return 1;
    }
    else if(num_param > 1 && args[0] == "TERMINATE") // Terminate a connection
    {
        if(std::isdigit((int)args[1][0]))
        {
            int sock = std::stoi(args[1]);
            std::string term = "_TERMINATE_";

            send_msg(sock,term);
            sleep(2);

            terminate_socket(sock);
        }
        else
            std::cout<<"Terminate requires argument, type help for assistance."<<std::endl;
        return 1;
    }
    else if(num_param > 1 && args[0] == "CONNECT") // Connect to a client
    {
        if(num_param < 3)
            std::cout<<"> Invalid Command format, type help for details"<<std::endl;
        else
            connect_client(args[1], args[2]);
        return 1;
    }
    else if(num_param > 1 && args[0] == "SEND") // Send a message to a client
    {
        if(num_param < 3)
            std::cout<<"> Invalid Command format, type help for details"<<std::endl;
        else
        {
            std::string msg = "_MESSAGE_,";
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
        }
        return 1;
    }
    else if(num_param > 1 && args[0] == "UPDATE")
    {
        int new_cost = 0;
        if((strcmp(args[3].c_str(), "inf") == 0) || (strcmp(args[3].c_str(), "INF") == 0))
            new_cost = INT_MAX;
        else
            new_cost = std::atoi(args[3].c_str());

        updateCost(std::atoi(args[1].c_str()), std::atoi(args[2].c_str()), new_cost);
        std::cout<<args[0] << " SUCCESS" << std::endl;
        return 1;
    }
    else // Output error for an unknown command
    {
        std::cout<<"> uknown command, use help for assistance"<<std::endl;
        return 1;
    }

    return 1; // Return 1 to keep program from crashing if something goes wrong here.
}

// **Help -- Displays available commands and their syntax
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

// **List all open connection for this host
void display()
{
    std::cout<<"ROUTING TABLE"<<std::endl;
    std::cout<<"Number Entries: " << my_table.num_neighbors << std::endl;
    for(unsigned int i = 0; i < my_table.num_neighbors; i++)
        std::cout << " " << my_table.neighbor_list[i].serv_id << "  " << my_table.neighbor_list[i].neighbor_id << "  " << my_table.neighbor_list[i].link_cost << std::endl;
    std::cout<<std::endl<<std::endl;

}

void list_connections()
{
    std::cout<<"> ID\tIP\t\tPORT"<<std::endl;

    for(int i = 0; i < list.client_list.size(); i++)
    {
        if(list.client_list[i] != -1)
        {
            std::cout<<"> "<<i<<"\t";
            std::cout<<list.client_list_ad[i]<<"\t";
            std::cout<<list.client_list_port[i]<<std::endl;
        }
    }

    std::cout<<"SERVER LIST"<<std::endl;
    std::cout<<"my_server_id: " << list.my_server_id << std::endl;
    for(unsigned int i = 0; i < my_table.num_servers; i++)
        std::cout<<" " << my_table.server_list[i].server_id << "  " << my_table.server_list[i].server_ipaddr << "  " << my_table.server_list[i].server_port << std::endl;
}

// ** Get Computer IP Address and returns value as string
// Code is modified from the following forum post
// http://stackoverflow.com/questions/212528/get-the-ip-address-of-the-machine
std::string get_myip()
{
    std::string my_ip;
    struct ifaddrs *myaddrs, *ifa;
    void *in_addr;

    // Gets list of ifaddrs structures for local network interfaces
    // http://man7.org/linux/man-pages/man3/getifaddrs.3.html
    if(getifaddrs(&myaddrs) != 0)
        std::cout<<"MYIP ERROR"<<std::endl;

    // Walk through the list of network interfaces ignoring anything that is not IPV4
    for (ifa = myaddrs; ifa != NULL; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == NULL)// Skip if pointer is null
            continue;

        if(ifa->ifa_addr->sa_family == AF_INET) // Display IPv4 Address
        {
            // Cast current network interface to sockaddr_in
            struct sockaddr_in *ip_addr = (struct sockaddr_in *)ifa->ifa_addr;

            char ipAddress[INET_ADDRSTRLEN]; // to Store IPAddress we found

            // Convert IPaddress to readable form
            // http://man7.org/linux/man-pages/man3/inet_ntop.3.html
            inet_ntop(AF_INET, &(ip_addr->sin_addr), ipAddress, INET_ADDRSTRLEN);// new line

            // Only display the interface if it is not the loopback address
            if (ipAddress[1] != '2')
            {
                std::string temp(ipAddress);
                my_ip = temp;//printf("> The IP address is: %s\n", ipAddress);
            }
        }
    }
    return my_ip;// Return the string value of the IPAddress
}

void connectNeighbors()
{
    for(unsigned int i = 0; i < my_table.num_neighbors; i++)
    {
        neighbor_info neighbors = my_table.neighbor_list[i];
        //std::cout<<"ME: " << list.my_server_id << " --- neighbor_serv: " << neighbors.serv_id << " --- neighbor_neighbor: " << neighbors.neighbor_id << std::endl;
        if(neighbors.serv_id == list.my_server_id && neighbors.neighbor_id != list.my_server_id)
        {
            //std::cout<<"Connecting to ---> " << neighbors.neighbor_id << std::endl;
            connect_client(my_table.server_list[i].server_ipaddr, std::to_string(my_table.server_list[i].server_port));
        }
    }
}

void updateCost(int server_id, int neighbor_id, int new_cost)
{
    int s_index = -1;
    int n_index = -1;

    for(unsigned int i = 0; i < my_table.num_neighbors; i++)
    {
        if(my_table.neighbor_list[i].serv_id == server_id)
        {
            s_index = i;

            if(my_table.neighbor_list[i].neighbor_id == neighbor_id)
            {
                n_index = i;
            }
        }
    }

    if(s_index == -1)
    {
        //std::cout<<"SERVER NOT IN TABLE"<<std::endl;
        neighbor_info new_neighbor;
        new_neighbor.serv_id = server_id;
        new_neighbor.neighbor_id = neighbor_id;
        new_neighbor.link_cost = new_cost;
        my_table.neighbor_list.push_back(new_neighbor);
        my_table.num_neighbors += 1;
    }
    else
    {
        if(n_index == -1)
        {
            //std::cout<<"NOT A NEIGHBOR OF THE SERVER"<<std::endl;
            neighbor_info new_neighbor;
            new_neighbor.serv_id = server_id;
            new_neighbor.neighbor_id = neighbor_id;
            new_neighbor.link_cost = new_cost;
            my_table.neighbor_list.push_back(new_neighbor);
            my_table.num_neighbors += 1;
        }
        else
        {
            //std::cout<<"SERVER AND NEIGHBOR PAIR FOUND.  UPDATE THE COST IN THE TABLE"<<std::endl;
            neighbor_info update;
            update = my_table.neighbor_list[n_index];
            update.link_cost = new_cost;
            my_table.neighbor_list[n_index] = update;
        }
    }
}

void increasePacketCount()
{
    list.packet_count++;
}

void resetPacketCount()
{
    list.packet_count = 0;
}

void crash()
{
    for(unsigned int i = 0; i < my_table.num_neighbors; i++)
    {
        neighbor_info my_list = my_table.neighbor_list[i];
        if(my_list.serv_id == list.my_server_id)
        {
            std::string msg = "_CRASH_,";
            my_list.link_cost = INT_MAX;
            msg += std::to_string(list.my_server_id);
            send_msg(my_list.neighbor_id,msg);
            terminate_socket(my_list.neighbor_id);
        }
    }
}

// **Connect to a client -- takes destination ip and port as parameters
void connect_client(std::string con_ip, std::string con_port)
{
    bool error_occured = false; // For flow control when errors occur
    int sockfd = 0; // File Descriptor for socket connection
    struct sockaddr_in client_info; // Struct to store connection information

    client_info.sin_family = AF_INET; // Sets connection for IPv4
    client_info.sin_port = htons(std::stoi(con_port)); // Converts port to network byte order and sets value

    std::string testval = get_myip(); // gets local machine ip
    if(strcmp(con_ip.c_str(), testval.c_str()) == 0) // Forbids self connection and sets error flag
    {
        //std::cout<<"> Self connection is forbidden."<<std::endl;
        error_occured = true;
    }
    else // Check to see if we already have connection to this client and sets error flag
        for(unsigned int i = 0; i < list.client_list_ad.size(); i++)
            if(strcmp(list.client_list_ad[i].c_str(), con_ip.c_str()) == 0)
            {
                error_occured = true;
                //std::cout<<"> Already connected to this client"<<std::endl;
            }

    if(!error_occured) // If no error at this point perform these operations
    {
        // http://man7.org/linux/man-pages/man2/socket.2.html
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Create new ipv4 TCP sockets
        {
            //perror("> Error : Could not create socket: ");
            error_occured = true; // if error occurs sets the flag
        }

        // http://man7.org/linux/man-pages/man3/inet_pton.3.html
        if(inet_pton(AF_INET, con_ip.c_str(), &client_info.sin_addr)<=0) // converts ip to network and stores value
        {
            //perror("> inet_pton error occured: ");
            error_occured = true; // if error occurs sets the flag
        }

        // http://man7.org/linux/man-pages/man2/connect.2.html
        if( connect(sockfd, (struct sockaddr *)&client_info, sizeof(client_info)) < 0) // open connection
        {
            //perror("> Connection Error: ");
            error_occured = true; // if error occurs sets the flag
        }

        if(!error_occured) // no error during the connecting phase then perform the following operations
        {
            std::string msg = list.listen_port; // Get the listening port for the local server
            int stat = send(sockfd, msg.c_str(), std::strlen(msg.c_str()), 0); // send port to client

            //Add Client to list of connected TCP Sockets
            list.client_list.push_back(sockfd);
            list.client_list_ad.push_back(con_ip);
            list.client_list_port.push_back(con_port);

            std::thread receive_func(server_Recv, sockfd); // Starts a receiving thread for this socket

            while(!receive_func.joinable()){} // wait for thread to become joinable/ or ready for operations
            receive_func.detach(); // Detach thread to run independently of main thread
        }
    }
}

// **Terminate the specified sockets -- takes index value(id) from list function
void terminate_socket(int sock)
{
    try
    {
        std::cout<<"sock param: " << sock << std::endl;
        int sock_fd;
        server_info the_server;

        for(unsigned int i = 0; i < my_table.num_servers; i++)
        {
            the_server = my_table.server_list[i];
            if(the_server.server_id == sock)
                break;
        }

        for(unsigned int i = 0; i < list.client_list.size(); i++)
        {
            if(strcmp(the_server.server_ipaddr.c_str(), list.client_list_ad[i].c_str()) == 0)
            {

                sock_fd = i;
                break;
            }
        }

        for(unsigned int i = 0; i < my_table.num_neighbors; i++)
        {
            neighbor_info to_update = my_table.neighbor_list[i];
            if(to_update.serv_id == list.my_server_id)
                if(to_update.neighbor_id == sock)
                    to_update.link_cost = INT_MAX;
        }

        int status = shutdown(list.client_list[sock_fd], SHUT_RDWR); // shutsdown read/write operations on socket
        if(status < 0)
            perror("> Shutdown Error: ");
        else
        {
            if(status == -1)
                fprintf(stderr,"> TERMINATE ERROR: %s\n",gai_strerror(status));
            else // Display message about closed connection
                std::cout<<"> Connection with "<< list.client_list_ad[sock_fd] << " has been closed" << std::endl;

            list.client_list[sock_fd] = -1; // Remove connection from list of open connections
            list.client_list_ad[sock_fd] = "";
            list.client_list_port[sock_fd] = "";
        }
    }
    catch(...)
    {
        std::cout<<"TERMINATE ERROR: SOCKET DOES NOT EXISTS"<<std::endl;
    }

}

// **Sends message to client -- takes server_id number from server_list
void send_msg(int sock, std::string msg)
{
    try
    {
        int sock_fd;
        server_info the_server;

        for(unsigned int i = 0; i < my_table.num_servers; i++)
        {
            the_server = my_table.server_list[i];
            if(the_server.server_id == sock)
                break;
        }

        for(unsigned int i = 0; i < list.client_list.size(); i++)
        {
            if(strcmp(the_server.server_ipaddr.c_str(), list.client_list_ad[i].c_str()) == 0)
            {
                sock_fd = i;
                break;
            }
        }

        int send_sock = list.client_list[sock_fd]; // Gets file descriptor for socket
        //std::cout<<"send_sock -> " << send_sock << std::endl;
        int status;
        const char *out = msg.c_str(); // Sets message for packet

        // http://man7.org/linux/man-pages/man2/send.2.html
        status = send(send_sock, msg.c_str(), std::strlen(msg.c_str()), 0); // Send packet to destination
        if(status < 0)
        {
            //fprintf(stderr,"> SEND ERROR: %s: ",gai_strerror(status));
            std::cout<<std::endl;
        }
    }
    catch(...)
    {
        std::cout<<"SEND ERROR: SOCKET DOES NOT EXIST"<<std::endl;
    }

}

void updateNeighbors()
{
    std::string update = create_packet(my_table.num_neighbors,std::atoi(list.listen_port.c_str()),get_myip(),my_table);
    //std::cout<<"Message: " << update << std::endl;
    for(unsigned int i = 0; i < my_table.num_servers; i++)
    {
        server_info this_server = my_table.server_list[i];
        if(isNeighbor(this_server.server_id) && this_server.server_id != list.my_server_id)
            send_msg(this_server.server_id,update);
    }
}

// ***************************************************************************
// ******************************Server Functions*****************************
// Start the server -- takes port number to listen on as parameter
int start_Server(std::string portNum)
{
    struct addrinfo hints,      //Hints - addrinfo struct to store connection configuration
                    *servinfo,  //ServInfo - Holds struct of connection information
                    *p;         //Walker pointer to iterate through servinfo struct
    int server_sock,            //Server_Sock - File descriptor that will be returned
            ret_code;           //ret_Code - hold return status from getaddrinfo() for errors


    memset(&hints, 0, sizeof hints);//Clear memory space for hints
    hints.ai_family = AF_INET;      //Sets IPV4
    hints.ai_socktype = SOCK_STREAM;//Sets TCP
    hints.ai_flags = AI_PASSIVE; // use my IP

    // Use getaddrinfo() to build networking information for server, stored in servinfo
    // http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    ret_code = getaddrinfo(NULL, portNum.c_str(), &hints, &servinfo);

    if (ret_code != 0) // Error occured
        fprintf(stderr, "> getaddrinfo: %s\n", gai_strerror(ret_code));

    for (p = servinfo; p != NULL; p = p->ai_next)// Walk through addrinfo struct
    {
        // Create server socket and store file descriptor in server_sock for
        // the current servinfo value
        server_sock = socket(p->ai_family, p->ai_socktype,p->ai_protocol);

        if (server_sock == -1) // Skip if socket cannot be created
            continue;

        if (bind(server_sock, p->ai_addr, p->ai_addrlen) == 0) // Bind socket to ip address and exit
        {
            break;
        }
    }

    if (p == NULL) // If no binding ever occured error occured
    {
        fprintf(stderr, "> Could not bind: ");
        exit(EXIT_FAILURE);
    }

    list.server = server_sock; // Save server file descriptor for socket

    return server_sock; // Return the socket file descriptor
}

// **Accept incoming connections -- takes server socket file descriptor and list of open connections
int server_Accept(int server_sock, std::vector<int> &client_list)
{
    struct sockaddr_in client; // Structure to hold client connection details
    int client_sock, status; // Status for return codes to handle errors
    char ip_string[INET6_ADDRSTRLEN]; // IP Address storage
    socklen_t len = sizeof(client); // Length of the client connection structure

    // Accept connection request and store connection details in client
    // http://man7.org/linux/man-pages/man2/accept.2.html
    client_sock = accept(server_sock, (struct sockaddr *) &client, &len);

    // Convert client ip to readable format
    inet_ntop(AF_INET, (struct sockaddr_in *)&client.sin_addr, ip_string, sizeof ip_string);

    list.client_list.push_back(client_sock); // Save connection file descriptor
    list.client_list_ad.push_back(ip_string);// Save the ip address of the client

    if(client_sock == -1) // Error occured and connection failed
        std::cout<<"a connection from client just failed."<<std::endl;
    else // Display connection message
        std::cout<<"> "<<ip_string<<" has been connected."<<std::endl;


    char *out; // Message Pointer

    memset(&out, 0, sizeof(out)); // Clear meory for message
    out = "";
    status = send(client_sock, &*out, strlen(out), 0); // Send welcome message to client
    if(status < 0) // Send error occured
    {
        fprintf(stderr,"> SEND ERROR: %s\n",gai_strerror(status));
        std::cout<<std::endl;
    }

    char in[255]; // Receive message
    size_t buff_size = 255; // Size of receive message
    int stat = 0; // For recv error

    memset(&in, 0, sizeof(in)); // Clear memory for receive
    while(1)
    {
        stat = recv(client_sock, in, buff_size, 0); // Receive response to welcome message

        if(stat > 1) // Receive message success
        {
            break;
        }
    }

    list.client_list_port.push_back(in); // Save port number from recv message

    std::thread receive_func(server_Recv, client_sock); // Start Receive thread

    while(!receive_func.joinable()){} // Wait for receive thread to be ready for operations
    receive_func.detach(); // Detach thread to run independant of main thread

    return 1; // Return success for accept.
}

void readTable(std::string file_name, route_table &myself)
{
    std::ifstream in_file;
    in_file.open(file_name, std::ios::in);

    if(in_file.fail())
        std::cout<<"File Does not exist!"<<std::endl;

    in_file >> myself.num_servers;
    in_file >> myself.num_neighbors;

    for(unsigned int i = 0; i < myself.num_servers; i++)
    {
        server_info new_server;
        in_file >> new_server.server_id;
        in_file >> new_server.server_ipaddr;
        in_file >> new_server.server_port;
        myself.server_list.push_back(new_server);
    }

    for(unsigned int i = 0; i < myself.num_neighbors; i++)
    {
        neighbor_info new_neighbor;
        in_file >> new_neighbor.serv_id;
        in_file >> new_neighbor.neighbor_id;
        in_file >> new_neighbor.link_cost;
        myself.neighbor_list.push_back(new_neighbor);
    }

    in_file.close();
}

std::string create_packet(int num_fields, int serv_port, std::string serv_ip, route_table the_routes)
{
    std::string message;

    message += std::to_string(num_fields);
    message += ",";
    message += std::to_string(serv_port);
    message += ",";
    message += serv_ip;

    for(unsigned int i = 0; i < the_routes.num_neighbors; i++)
    {
        neighbor_info neighbors = the_routes.neighbor_list[i];
        server_info servers = getNeighborInfo(neighbors.serv_id);

        message += ",";
        message += servers.server_ipaddr;
        message += ",";
        message += std::to_string(servers.server_port);
        message += ",";
        message += std::to_string(neighbors.neighbor_id);
        message += ",";
        message += std::to_string(neighbors.link_cost);
    }

    return message;
}

int getServerInfo()
{
    std::string my_ip = get_myip();
    int index = -1;

    for(unsigned int i = 0; i < my_table.num_servers; i++)
    {
        server_info the_server = my_table.server_list[i];
        if(my_ip.compare(the_server.server_ipaddr) == 0)
        {
            index = i;
        }
    }
    return index;
}

bool isNeighbor(int id)
{
    bool ret_val = false;

    for(unsigned int i = 0; i < my_table.num_neighbors; i++)
    {
        neighbor_info my_list = my_table.neighbor_list[i];

        if(my_list.serv_id == list.my_server_id)
            if(my_list.neighbor_id == id)
                ret_val = true;
    }
    return ret_val;
}

server_info getNeighborInfo(int neighbor_id)
{
    server_info the_server;
    for(unsigned int i = 0; i < my_table.num_servers; i++)
    {
        server_info temp = my_table.server_list[i];
        if(temp.server_id == neighbor_id)
        {
            the_server = temp;
            break;
        }
    }
    return the_server;
}

int getServerbyIP(std::string ip)
{
    int id;
    for(unsigned int i = 0; i < my_table.num_servers; i++)
    {
        server_info temp = my_table.server_list[i];
        if(strcmp(ip.c_str(), temp.server_ipaddr.c_str()) == 0)
            id = temp.server_id;
    }
    return id;
}
