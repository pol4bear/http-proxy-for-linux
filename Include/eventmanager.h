# pragma once

# include <iostream>
# include <string>
# include <arpa/inet.h>

class EventManager {
public:
    // Event
    static void onNewClient(int client_socket, sockaddr client_address);
    static void onClientDisconnect(int client_socket);
    static void onNewRequestReceived(int client_socket, std::string message);
    static void onNewResponseSent(int client_socket, std::string message);

private:
    // Constant
    static const std::string info;
    static const std::string positive;
    static const std::string negative;
    static const std::string error;

    // I/O Method
    static void newMessage(std::string prefix, std::string message);
};
