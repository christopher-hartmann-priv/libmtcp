#pragma once

#include "../tcp_server.hpp"

namespace miniTCP
{
  inline TCPServer::TCPServer(const std::string &ip, const uint16_t port, const bool autoRepair)
      : TCPSocket(ip, port, autoRepair)
  {
  }

  inline TCPServer::~TCPServer()
  {
    try
    {
      close();
    }
    catch (...)
    {
      // catch everything
    }
  }

  inline OpenResult TCPServer::openImpl()
  {
    SOCKET listener = -1;

    // for cleanup
    auto unlistener = [&listener](void *)
    {
      if (listener > -1)
      {
#ifdef _WIN32
        ::shutdown(listener, SD_BOTH);
        ::closesocket(listener);
#else
        ::shutdown(listener, SHUT_RDWR);
        ::close(listener);
#endif

        listener = -1;
      }
    };

    // for cleanup
    auto freeAddr = [](struct addrinfo *p)
    {
      if (p)
        freeaddrinfo(p);
    };

    // we clean our port before open (in case it has been used before)
    TCPServer::close();

    struct addrinfo hints;
    std::memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *tmp;
    if (::getaddrinfo(_ip.c_str(), std::to_string(_port).c_str(), &hints, &tmp) != 0)
      return OpenResult(OpenCode::FAILURE, StepCode::GETADDRINFO, getErrorCode());

    auto result = std::shared_ptr<struct addrinfo>(tmp, freeAddr);
    listener = ::socket(result->ai_family, result->ai_socktype, result->ai_protocol);

    if (listener < 0)
      return OpenResult(OpenCode::FAILURE, StepCode::SOCKET, getErrorCode());

    // a guard to make sure the listener dies on return
    auto guard = std::shared_ptr<void>(nullptr, unlistener);

    if (!tweakSocketConfig(listener))
      return OpenResult(OpenCode::FAILURE, StepCode::SETSOCKOPT, getErrorCode());

    if (::bind(listener, result->ai_addr, static_cast<int>(result->ai_addrlen)) != 0)
      return OpenResult(OpenCode::FAILURE, StepCode::BIND, getErrorCode());

    const int singleConnection = 1;
    if (::listen(listener, singleConnection) != 0)
      return OpenResult(OpenCode::FAILURE, StepCode::LISTEN, getErrorCode());

    _listener = listener; // we need to store it, so accept can be aborted (via TCPServer::close())

    auto s = ::accept(_listener, nullptr, nullptr);
    _listener = -1;

// TODO check for windows
#ifdef _WIN32
    auto timeoutErrno = 11;
#else
    auto timeoutErrno = 11;
#endif

    if (s < 0)
      return OpenResult(getErrorCode() == timeoutErrno ? OpenCode::TIMEOUT : OpenCode::FAILURE, StepCode::ACCEPT, getErrorCode());

    _socket = s;
    _connected = true;

    return OpenResult(OpenCode::OK, StepCode::UNKNOWN, getErrorCode());
  }

  inline void TCPServer::close()
  {
    // close 'established connection'
    TCPSocket::close();

    // close 'listen'
    if (_listener > -1)
    {
#ifdef _WIN32
      ::shutdown(_listener, SD_BOTH);
      ::closesocket(_listener);
#else
      ::shutdown(_listener, SHUT_RDWR);
      ::close(_listener);
#endif
      _listener = -1;
    }
  }
}