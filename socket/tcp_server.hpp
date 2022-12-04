#pragma once

#include "../socket/_detail/tcp_socket.hpp"

namespace miniTCP
{
  class TCPServer : public TCPSocket
  {
  public:
    TCPServer(const std::string &ip, const uint16_t port, const bool autoRepair = false);
    virtual void close() override;

    virtual ~TCPServer();

  private:
    SOCKET _listener = -1;

    virtual OpenResult openImpl() override;
  };
}

#include "../socket/_detail/tcp_server_detail.hpp"
