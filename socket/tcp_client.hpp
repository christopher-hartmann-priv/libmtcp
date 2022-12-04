#pragma once

#include <chrono>

#include "../socket/_detail/tcp_socket.hpp"

namespace miniTCP
{
  class TCPClient : public TCPSocket
  {
  public:
    TCPClient(const std::string &ip, const uint16_t port, const bool autoRepair = false);
    virtual ~TCPClient() = default;
    OpenResult open(std::chrono::milliseconds timeout, std::chrono::milliseconds retry = std::chrono::milliseconds(50));

  private:
    virtual OpenResult open() override
    {
      return TCPSocket::open();
    }

    virtual OpenResult openImpl() override;
  };
}

#include "../socket/_detail/tcp_client_detail.hpp"