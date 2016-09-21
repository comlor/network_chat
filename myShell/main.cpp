#include <iostream>
#include <iomanip>
#include <vector>

using namespace std;

// ****************************Global Variables****************************
int NUM_ARGS = 5;
// ************************************************************************

// **************************Function Prototypes***************************
// Core Functions
void main_loop(void);
string read_input(void);
vector<string> parse_input(string &);
int ex_args(vector<string>);

// Parameter Functions
void help(); // Done
void myIP();
void myPort();
void connect(string dest, int port);
void listConnection();
void terminate(int connect_id);
void send(int connect_id, string message);
void exit();

// ************************************************************************

// ******************************Main Function********************************
int main(int argc, char *argv[])
{
    main_loop();

    std::cout<<"EXIT SUCCESS"<<std::endl;
    return EXIT_SUCCESS;
}
// ***************************************************************************

// ****************************Main Program Loop******************************
void main_loop(void)
{
    string input;
    vector<string> the_args;
    int status = 1;

    while(status)
    {
        std::cout<<"> ";
        input = read_input();
        the_args = parse_input(input);
        status = ex_args(the_args);
    }
}
// ***************************************************************************

// *****************************Read Console Input****************************
string read_input(void)
{
    string in;
    getline(std::cin, in);
    return in;
}
// ***************************************************************************

// *********************Parse Parameters from Console Input*******************
vector<string> parse_input(string &in)
{
    int num_param = 1;

    for(int i = 0; i < in.length(); i++)
    {
        if(in[i] == ' ')
            num_param++;
    }

    NUM_ARGS = num_param;
    vector<string> param;
    param.resize(num_param);
    int end = 0, index = 0;

    for(int i = 0; i < in.length(); i++)
    {
        end = in.find_first_of(' ', i);

        if(end == -1)
        {
            string insert = in.substr(i);
            param[index] = insert;
            for(int j = 0; j < param[index].length(); j++)
                param[index][j] = toupper(param[index][j]);
            break;
        }
        else
        {
            string insert = in.substr(i, end - i);
            param[index] = insert;
            for(int j = 0; j < param[index].length(); j++)
                param[index][j] = toupper(param[index][j]);
        }

        i = end;
        index++;
    }
    return param;
}
// ***************************************************************************

// ****************************Execute Parameters*****************************
int ex_args(vector<string> args)
{
    int num_param = NUM_ARGS;

    for(int i = 0; i < num_param; i++)
        cout<<"param check: " << args[i] << endl;

    if(num_param == 1 && args[0] == "EXIT" )
        return 0;
    else if(num_param == 1 && args[0] == "HELP" )
        help(); return 1;
}
// ***************************************************************************

void help()
{
    cout<<"****************************************************************** "<<endl;
    cout<<"-------------------------Chat Application------------------------- "<<endl;
    cout<<"****************************************************************** "<<endl;
    cout<<endl;
    cout<<"Chat is a communication application to communicate with other      "<<endl;
    cout<<"users and view network connection data using TCP connection        "<<endl;
    cout<<endl                                                                 <<endl;
    cout<<"Command List:                                                      "<<endl;
    cout<<"help - displays this help document"                           <<endl<<endl;
    cout<<"myip - displays your network ip address"                      <<endl<<endl;
    cout<<"myport - displays the port number used "                      <<endl<<endl;
    cout<<"connect <destination> <port no> - creates new connection to"        <<endl;
    cout<<"      specified ip and port"                                  <<endl<<endl;
    cout<<"list - lists all the open connections this process is part of"<<endl<<endl;
    cout<<"terminate <connection id> - terminates the connection of the       "<<endl;
    cout<<"      specified id number                                    "<<endl<<endl;
    cout<<"send <connection id> <message> - sends a message to the            "<<endl;
    cout<<"      specified connection id.                               "<<endl<<endl;
    cout<<"exit - Close all connection and terminate program            "<<endl<<endl;
    cout<<"****************************************************************** "<<endl;
}
