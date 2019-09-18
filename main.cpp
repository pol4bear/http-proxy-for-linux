# include <iostream>
# include <signal.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include "Include/webproxy.h"

using namespace std;


/***** Constant *****/
static const string info = "[i]";
static const string positive = "[+]";
static const string negative = "[-]";
static const string error = "[!]";


/***** I/O Method *****/
static void newMessage(string prefix, string message)
{
    cout << prefix << " " << message << "\n";
}


/***** Callback *****/
static void onNewClient(int client_socket, sockaddr client_address)
{
    in_addr client_ip = reinterpret_cast<sockaddr_in*>(&client_address)->sin_addr;
    newMessage(positive, "New connection from " + string(inet_ntoa(client_ip)) + " started at socket " + to_string(client_socket));
}

static void onClientDisconnect(int client_socket)
{
    newMessage(negative, "Socket " + to_string(client_socket) + " Disconnected from server");
}


static void onNewRequestReceived(int client_socket, string message)
{
    newMessage(positive, "New request from socket " + to_string(client_socket) + " " + message);
}

static void onNewResponseSent(int client_socket, string message)
{
    newMessage(positive, "Response for " + message + " sent to socket " + to_string(client_socket) + " successfully");
}


/***** Main *****/
int main(int argc, char *argv[])
{
    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " [port]\n";
        exit(1);
    }

    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    // Get parameter and initialize variable
    uint16_t port;
    in_addr_t source_address = INADDR_ANY;
    int max_client = TcpServer::DEFAULT_CLIENT_COUNT;

    WebProxy web_proxy;
    // Set callback
    web_proxy.setSendNewClientConnected(onNewClient);
    web_proxy.setSendClientDisconnected(onClientDisconnect);
    web_proxy.setSendNewRequestReceived(onNewRequestReceived);
    web_proxy.setSendNewResponseSent(onNewResponseSent);

    newMessage(info, "Web Proxy Initialized");

    try {
        if (argc == 2) {
            port = uint16_t(stoi(argv[1]));
        }
        else {
            if (inet_pton(AF_INET, argv[1], &source_address) < 1) {
                cerr << "Usage: " << argv[0] << "[source IP address] [port]\n";
                exit(1);
            }

            port = uint16_t(stoi(argv[2]));

            if (argc > 4) {
                max_client = stoi(argv[3]);
            }
        }

        web_proxy.start(port, source_address, max_client);

        newMessage(info, "Web Proxy Started");
    }
    catch (runtime_error e) {
        newMessage(error, e.what());
        exit(1);
    }
    catch (invalid_argument e) {
        newMessage(error, "Wrong argument type");
        exit(1);
    }

    while(web_proxy.isStarted()) {}

    newMessage(info, "Web Proxy Stopped");

    exit (0);
}
