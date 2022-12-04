#pragma once

#include <stdint.h>
#include <stddef.h>

namespace miniTCP
{
  enum class IOCode : int
  {
    OK = 0,
    TIMEOUT = -1,
    ERROR = -2,
    PEER_SHUTDOWN = -3,
    NOT_CONNECTED = -4,
    UNKNOWN = -5
  };

  /**
   * @brief Describes the operation, which was attempted.
   */
  enum class OPCode : int
  {
    READ = 0,
    READ_WAIT_ALL,
    WRITE,
    UNKNOWN
  };

  struct IOResult
  {
  public:
    IOResult(size_t size, OPCode op, IOCode code, int error)
        : _size(size), _op(op), _code(code), _error(error)
    {
      // nothing to do here
    }

    size_t _size = 0;
    OPCode _op = OPCode::UNKNOWN;
    IOCode _code = IOCode::UNKNOWN;
    int _error = 0;
  };

  enum class OpenCode : int
  {
    OK = 0,
    FAILURE = -1,
    UNKNOWN = -2,
    TIMEOUT = -3
  };

  enum class StepCode : uint8_t
  {
    SOCKET = 0,
    BIND,
    CONNECT,
    ACCEPT,
    LISTEN,
    GETADDRINFO,
    SETSOCKOPT,
    UNKNOWN
  };

  struct OpenResult
  {
  public:
    OpenResult(OpenCode op, StepCode step, int error)
        : _op(op), _step(step), _error(error)
    {
      // nothing to do here
    }

    OpenCode _op = OpenCode::UNKNOWN;
    StepCode _step = StepCode::UNKNOWN;
    int _error = 0;
  };
}