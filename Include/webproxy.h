# pragma once

# include "Include/tcpserver.h"

class WebProxy {
public:
    WebProxy();

private:
    TcpServer tcp_server;
};
