#pragma once

#include "tcp_socket.hpp"

namespace miniTCP
{
  inline void TCPSocket::init()
  {
#ifdef _WIN32
    static atomic_bool isInit = false;
    if (isInit)
      return;

    static WSADATA wsa;
    isInit = WSAStartup(MAKEWORD(2, 2), &wsa);
#endif
  }

  inline TCPSocket::TCPSocket(const std::string &ip, const uint16_t port, const bool autoRepair)
      : _ip(ip), _port(port), _repairOnError(autoRepair)
  {
    // wsa init
    TCPSocket::init();
  }

  inline TCPSocket::~TCPSocket()
  {
    try
    {
      close();
    }
    catch (...)
    {
      // we just catch everything
    }
  }

  inline IOResult TCPSocket::write(const void *data, const size_t size)
  {
    if (_repairOnError && !isConnected())
      open();

    if (!isConnected())
      return IOResult(0, OPCode::WRITE, IOCode::NOT_CONNECTED, TCPSocket::getErrorCode());

    const int flags = 0;
    int bytes = 0;

    {
      std::lock_guard lock(_mutexWrite);
      bytes = ::send(_socket, data, size, flags);
      if (bytes < 0)
      {
        _connected = false;
        return IOResult(0, OPCode::WRITE, IOCode::ERROR, TCPSocket::getErrorCode());
      }
    }

    return IOResult(bytes, OPCode::WRITE, IOCode::OK, 0);
  }

  inline IOResult TCPSocket::read(void *data, const size_t size, const bool waitAll)
  {
    const auto op = waitAll ? OPCode::READ_WAIT_ALL : OPCode::READ;
    if (_repairOnError && !isConnected())
      open();

    if (!isConnected())
      return IOResult(0, op, IOCode::NOT_CONNECTED, TCPSocket::getErrorCode());

    int bytesRead = 0;
    int bytesReadTotal = 0;

    {
      std::lock_guard lock(_mutexRead);

      do
      {
        bytesRead = ::recv(_socket, reinterpret_cast<char *>(data) + bytesReadTotal, size - bytesReadTotal, waitAll ? MSG_WAITALL : 0);

        if (bytesRead == 0) // peer closed connection
        {
          _connected = false;
          return IOResult(bytesReadTotal, op, IOCode::PEER_SHUTDOWN, 0);
        }
        else if (bytesRead < 0) // a socket error occured
        {
          _connected = false;
          return IOResult(bytesReadTotal, op, IOCode::ERROR, getErrorCode());
        }

        bytesReadTotal += bytesRead;

      } while (bytesReadTotal < size);
    }

    return IOResult(bytesReadTotal, op, IOCode::OK, 0);
  }

  inline void TCPSocket::close()
  {
    auto flush = [this]
    {
      std::vector<char> dummy(100, 0);
      while (::recv(_socket, dummy.data(), dummy.size(), 0) > 0)
        ;
    };

    if (_socket > -1)
    {
#ifdef _WIN32
      ::shutdown(_socket, SD_BOTH);
      flush();
      ::closesocket(_socket);
#else
      ::shutdown(_socket, SHUT_RDWR);
      flush();
      ::close(_socket);
#endif
      _socket = -1;
    }
  }

  inline OpenResult TCPSocket::open()
  {
    // we clean our port before open (in case it has been used before)
    close();
    std::lock_guard lr(_mutexRead);
    std::lock_guard lw(_mutexWrite);
    return openImpl();
  }

  inline bool TCPSocket::isConnected() const
  {
    return _connected && _socket > -1;
  }

  inline int TCPSocket::getErrorCode()
  {
#ifndef _WIN32
    return errno;
#else
    return WSAGetLastError();
#endif
  }

  inline int TCPSocket::tweakSocketConfig(SOCKET s)
  {
    // timeouts
    {
      struct timeval t;

      // set to 5 seconds
      {
        std::memset(&t, 0, sizeof(t));
        t.tv_sec = 5;
        t.tv_usec = 0;
      }

      if (::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &t, sizeof(t)) < 0)
        return false;

      if (::setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, &t, sizeof(t)) < 0)
        return false;
    }

    // buffer sizes
    {
      const int more = 1024 * 1024;
      if (::setsockopt(s, SOL_SOCKET, SO_SNDBUF, &more, sizeof(more)) < 0)
        return false;

      if (::setsockopt(s, SOL_SOCKET, SO_RCVBUF, &more, sizeof(more)) < 0)
        return false;
    }

    // address reuse
    {
      const int yes = 1;
      if (::setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
        return false;

      // #ifdef SO_REUSEPORT
      //     if (setsockopt(s, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) < 0)
      //       ; // bad
      // #endif
    }
    return true;
  }
}
