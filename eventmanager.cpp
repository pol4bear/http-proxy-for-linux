# include "Include/eventmanager.h"

using namespace std;

/***** Constant *****/
const string EventManager::info = "[i]";
const string EventManager::positive = "[+]";
const string EventManager::negative = "[-]";
const string EventManager::error = "[!]";

/***** Event *****/
void EventManager::onNewClient(int client_socket, sockaddr client_address)
{
    in_addr client_ip = reinterpret_cast<sockaddr_in*>(&client_address)->sin_addr;
    newMessage(positive, "New connection from " + string(inet_ntoa(client_ip)) + " started at socket " + to_string(client_socket));
}

void EventManager::EventManager::onClientDisconnect(int client_socket)
{
    newMessage(negative, "Socket " + to_string(client_socket) + " Disconnected from server");
}


void EventManager::EventManager::onNewRequestReceived(int client_socket, string message)
{
    newMessage(positive, "New request from socket " + to_string(client_socket) + " " + message);
}

void EventManager::EventManager::onNewResponseSent(int client_socket, string message)
{
    newMessage(positive, "Response for " + message + " sent to socket " + to_string(client_socket) + " successfully");
}


/***** I/O Method *****/
void EventManager::EventManager::newMessage(string prefix, string message)
{
    cout << prefix << " " << message << "\n";
}
