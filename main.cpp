# include <iostream>
# include <signal.h>
# include <stdlib.h>
# include <arpa/inet.h>
# include "Include/webproxy.h"
# include "Include/eventmanager.h"


using namespace std;


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
    web_proxy.setSendNewClientConnected(EventManager::onNewClient);
    web_proxy.setSendClientDisconnected(EventManager::onClientDisconnect);
    web_proxy.setSendNewRequestReceived(EventManager::onNewRequestReceived);
    web_proxy.setSendNewResponseSent(EventManager::onNewResponseSent);

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
