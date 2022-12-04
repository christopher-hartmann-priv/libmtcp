#pragma once

#include <stdlib.h>
#include <string>   // for stdstring
#include <stdint.h> // for assorted ints
#include <atomic>   // for atomic
#include <memory>   // for shared_ptr
#include <cstring>  // for memset
#include <vector>
#include <mutex>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")
#pragma comment(lib, "AdvApi32.lib")
#else
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#endif

#include "tcp_result.hpp"

namespace miniTCP
{
  class TCPSocket
  {
  public:
    TCPSocket(const std::string &ip, const uint16_t port, const bool autoRepair);
    bool isConnected() const;

    virtual void close();
    virtual OpenResult open();

    IOResult write(const void *data, const size_t size);
    IOResult read(void *data, const size_t size, const bool waitAll);

    virtual ~TCPSocket();

  protected:
#ifndef _WIN32
    using SOCKET = int;
#endif
    SOCKET _socket = -1;

    bool _repairOnError = false;
    std::atomic_bool _connected = false;

    std::string _ip = "127.0.0.1";
    uint16_t _port = 32768;

    /**
     * @brief This mutex blocks the read and open methods
     */
    std::recursive_mutex _mutexRead;

    /**
     * @brief This mutex blocks the write and open methods
     */
    std::recursive_mutex _mutexWrite;

  protected:
    static int getErrorCode();
    static int tweakSocketConfig(SOCKET s);

  private:
    static void init();
    virtual OpenResult openImpl() = 0;
  };
}
#include "../_detail/tcp_socket_detail.hpp"