# include "Include/tcpserver.h"

using namespace std;


/***** Constructor *****/
TcpServer::TcpServer()
    : is_started(false), max_client(DEFAULT_CLIENT_COUNT),
      on_new_client(nullptr), on_client_disconnected(nullptr),
      on_payload_received(nullptr), on_error(nullptr),
      on_warning(nullptr)
{
}

TcpServer::~TcpServer()
{
    stop();
}

bool TcpServer::isStarted()
{
    return is_started;
}


/***** Property *****/
void TcpServer::setOnNewClient(const function<void (int, sockaddr)> &method_in)
{
    on_new_client = method_in;
}

void TcpServer::setOnClientDisconnected(const function<void (int)> &method_in)
{
    on_client_disconnected = method_in;
}

void TcpServer::setOnPayloadReceived(const function<void (int, const uint8_t *, const ssize_t)> &method_in)
{
    on_payload_received = method_in;
}

void TcpServer::setOnError(const function<void (string)> &method_in)
{
    on_error = method_in;
}

void TcpServer::setOnWarning(const function<void (string)> &method_in)
{
    on_warning = method_in;
}

/***** Public Method *****/
TcpServer::Status::Id TcpServer::start(uint16_t port_in)
{
    // Check proxy is already started
    if (is_started)
        return Status::BAD_REQUEST;

    // Initialize local variable
    is_started = true;
    port = port_in;
    source_address = INADDR_ANY;
    max_client = DEFAULT_CLIENT_COUNT;

    return startServer();
}

TcpServer::Status::Id TcpServer::start(uint16_t port_in, in_addr_t source_address_in)
{
    // Check proxy is already started
    if (is_started)
        return Status::BAD_REQUEST;

    // Initialize local variable
    is_started = true;
    port = port_in;
    source_address = source_address_in;
    max_client = DEFAULT_CLIENT_COUNT;

    return startServer();
}

TcpServer::Status::Id TcpServer::start(uint16_t port_in, int max_client_in)
{
    // Check proxy is already started
    if (is_started)
        return Status::BAD_REQUEST;

    // Initialize local variable
    is_started = true;
    port = port_in;
    source_address = INADDR_ANY;
    max_client = max_client_in;

    return startServer();
}

TcpServer::Status::Id TcpServer::start(uint16_t port_in, in_addr_t source_address_in, int max_client_in)
{
    // Check proxy is already started
    if (is_started)
        return Status::BAD_REQUEST;

    // Initialize local variable
    is_started = true;
    port = port_in;
    source_address = source_address_in;
    max_client = max_client_in;

    return startServer();
}

TcpServer::Status::Id TcpServer::stop() {
    // Check proxy is already stopped
    if (!is_started)
        return Status::BAD_REQUEST;

    is_started = false;

    destroyRelaySocket();
}

void TcpServer::disconnectClient(int client_socket)
{
    close(client_socket);

    if (clients.find(client_socket) != clients.end())
        clients.erase(client_socket);

    onClientDisconnected(client_socket);
}

ssize_t TcpServer::sendToClient(int client_socket, const uint8_t *payload, ssize_t payload_size)
{
    if (clients.find(client_socket) != clients.end())
        return send(client_socket, payload, static_cast<size_t>(payload_size), 0);

    return -1;
}


/***** Private Method *****/
TcpServer::TcpServer::Status::Id TcpServer::startServer()
{
    Status::Id status;

    // Initialize Socket
    if ((status = initializeRelaySocket()) != Status::SUCCESS)
        return status;

    // Start connection thread
    connect_thread = thread(&TcpServer::connectAction, this);
}


/***** Callback *****/
void TcpServer::onNewClient(int client_socket, sockaddr client_address)
{
    if (on_new_client != nullptr)
        on_new_client(client_socket, client_address);
}

void TcpServer::onClientDisconnected(int client_socket)
{
    if (on_client_disconnected != nullptr)
        on_client_disconnected(client_socket);
}

void TcpServer::onPayloadReceived(int client_socket, const uint8_t *payload, const ssize_t receive_size)
{
    if (on_payload_received != nullptr)
        on_payload_received(client_socket, payload, receive_size);
}

void TcpServer::onError(string message)
{
    if (on_error != nullptr)
        on_error(message);
}

void TcpServer::onWarning(string message)
{
    if (on_warning != nullptr)
        on_warning(message);
}


/***** Private Method *****/
TcpServer::Status::Id TcpServer::initializeRelaySocket()
{
    // Create socket
    relay_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (relay_socket < 0)
        return Status::FAIL_SOCKET_CREATE;

    // Enable bind another socket
    int option;
    option = 1;
    setsockopt(relay_socket, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // Initialize socket address
    sockaddr_in server_address;
    server_address.sin_addr.s_addr = source_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);

    // Bind socket
    if(bind(relay_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) < 0)
        return Status::FAIL_SOCKET_BIND;

    return Status::SUCCESS;
}

void TcpServer::destroyRelaySocket()
{
    close(relay_socket);
}


/***** Thread Action *****/
void TcpServer::connectAction()
{
    while (is_started) {
        // Listen new connection from socket
        if (listen(relay_socket, max_client) < 0) {
            onError("Cannot listen from socket");
            break;
        }

        // Accept connection from socket
        sockaddr_in client_address_in;
        int client_address_size = sizeof(client_address_in);
        int client_socket = accept(relay_socket, reinterpret_cast<sockaddr*>(&client_address_in),
                                   reinterpret_cast<socklen_t*>(&client_address_size));
        if(client_socket < 0) {
            onWarning("Client socket accept failed");
            continue;
        }

        sockaddr *client_address = reinterpret_cast<sockaddr*>(&client_address_in);

        clients[client_socket] = Client(*client_address, true);

        onNewClient(client_socket, *client_address);

        thread client_thread(&TcpServer::listenAction, this, client_socket);

        client_thread.detach();
    }
}

void TcpServer::listenAction(int client_socket)
{
    while (is_started) {
        uint8_t buffer[1500];
        ssize_t receive_size = 0;

        memset(&buffer, 0, 1500);

        if((receive_size = recv(client_socket, buffer, sizeof(buffer), 0)) > 0)
            onPayloadReceived(client_socket, reinterpret_cast<uint8_t*>(buffer), receive_size);
        else if(receive_size < 0){
            disconnectClient(client_socket);
            break;
        }
        else {
            onWarning("Receive failed from client socket " + to_string(client_socket));
            disconnectClient(client_socket);
            break;
        }
    }
}
