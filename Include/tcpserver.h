# pragma once

# include <cstdint>
# include <functional>
# include <map>
# include <stdexcept>
# include <cstdlib>
# include <string.h>
# include <thread>
# include <unistd.h>
# include <arpa/inet.h>
# include <netinet/in.h>
# include <sys/socket.h>

class Client {
public:
    Client() : is_connected(false)
    {
    }
    Client(sockaddr address_in, bool is_connected_in = false) :
        address(address_in), is_connected(is_connected_in)
    {
    }

    sockaddr address;
    bool is_connected;
};

class TcpServer
{
public:
    TcpServer();
    ~TcpServer();

    // Constant
    static constexpr int DEFAULT_CLIENT_COUNT = 50;
    static constexpr int MAX_TRANSMISSION_UNIT = 1446;

    // Status code
    class Status {
    public:
        enum Id {
            SUCCESS = 0,
            BAD_REQUEST,
            FAIL_SOCKET_CREATE,
            FAIL_SOCKET_BIND
        };
    };

    // Property
    bool isStarted();
    void setOnNewClient(const std::function<void(int, sockaddr)> &method_in);
    void setOnClientDisconnected(const std::function<void(int)> &method_in);
    void setOnPayloadReceived(const std::function<void (int, const uint8_t*, const ssize_t)> &method_in);
    void setOnError(const std::function<void (std::string)> &method_in);
    void setOnWarning(const std::function<void (std::string)> &method_in);


    // Public Method
    Status::Id start(uint16_t port_in);
    Status::Id start(uint16_t port_in, in_addr_t source_address_in);
    Status::Id start(uint16_t port_in, int max_client_in);
    Status::Id start(uint16_t port_in, in_addr_t source_address_in, int max_client_in);
    Status::Id stop();
    void disconnectClient(int client_socket);
    ssize_t sendToClient(int client_socket, const uint8_t *payload, ssize_t payload_size);

private:
    // Private Member
    bool is_started;
    uint16_t port;
    in_addr_t source_address;
    int max_client;
    int relay_socket;
    std::map<int, Client> clients;

    // Private Method
    Status::Id startServer();

    // Callback
    std::function<void(int, sockaddr)> on_new_client;
    void onNewClient(int client_socket, sockaddr client_address);
    std::function<void(int)> on_client_disconnected;
    void onClientDisconnected(int client_socket);
    std::function<void(int, const uint8_t*, const ssize_t)> on_payload_received;
    void onPayloadReceived(int client_socket, const uint8_t *payload, const ssize_t payload_size);
    std::function<void(std::string)> on_error;
    void onError(std::string message);
    std::function<void(std::string)> on_warning;
    void onWarning(std::string message);

    // Thread
    std::thread connect_thread;

    // Thread Action
    void connectAction();
    void listenAction(int client_socket);

    // Private Method
    Status::Id initializeRelaySocket();
    void destroyRelaySocket();
};
