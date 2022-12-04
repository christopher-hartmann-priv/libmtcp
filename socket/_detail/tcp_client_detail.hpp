#pragma once

#include <thread>

#include "../tcp_client.hpp"

namespace miniTCP
{
  inline TCPClient::TCPClient(const std::string &ip, const uint16_t port, const bool autoRepair)
      : TCPSocket(ip, port, autoRepair)
  {
    // nothing to do here
  }

  inline OpenResult TCPClient::openImpl()
  {
    close();

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    // Resolve the server address and port
    struct addrinfo *tmp;
    if (::getaddrinfo(_ip.c_str(), std::to_string(_port).c_str(), &hints, &tmp) != 0)
      return OpenResult(OpenCode::FAILURE, StepCode::GETADDRINFO, getErrorCode());

    auto freeAddr = [](struct addrinfo *p)
    {
      if (p)
        freeaddrinfo(p);
    };

    auto result = std::shared_ptr<struct addrinfo>(tmp, freeAddr);

    for (auto ptr = result.get(); ptr != NULL; ptr = ptr->ai_next)
    {
      SOCKET s = -1;

      // socket create
      {
        s = ::socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        if (s < 0)
          return OpenResult(OpenCode::FAILURE, StepCode::SOCKET, 0);
      }

      if (!tweakSocketConfig(s))
        return OpenResult(OpenCode::FAILURE, StepCode::SETSOCKOPT, 0);

      // connect
      if (::connect(s, ptr->ai_addr, (int)ptr->ai_addrlen) != 0)
      {
#ifdef _WIN32
        ::closesocket(s);
#else
        ::close(s);
#endif
        if (!ptr->ai_next)
          return OpenResult(OpenCode::FAILURE, StepCode::CONNECT, getErrorCode());

        continue;
      }

      _socket = s;
      _connected = true;
      break;
    }
    return OpenResult(OpenCode::OK, StepCode::UNKNOWN, 0);
  }

  OpenResult TCPClient::open(std::chrono::milliseconds timeout, std::chrono::milliseconds retry)
  {
    using namespace std::chrono;
    const auto end = system_clock::now() + timeout;

    OpenResult r(OpenCode::UNKNOWN, StepCode::UNKNOWN, 0);

    do
    {
      r = TCPSocket::open();
      if (r._op == OpenCode::OK)
        return r;

      std::this_thread::sleep_for(retry);
    } while (end > system_clock::now());

    r._op = OpenCode::TIMEOUT;
    return r;
  }

}
