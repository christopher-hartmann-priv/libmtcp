#include <vector>
#include <iostream>
#include <iomanip>

#include "../socket/tcp_server.hpp"

bool handleError(const miniTCP::IOResult &result, const bool printOk = false)
{
  using namespace miniTCP;
  using c = IOCode;

  switch (result._code)
  {
  default:
    std::cout << "this should not happen.\n";
    return true;
  case (c::OK):
    if (printOk)
      std::cout << "OK " << static_cast<int>(result._op) << ' ' << std::setw(8) << result._size << '\n';
    return false;
  case (c::TIMEOUT):
    std::cout << "TO " << static_cast<int>(result._op) << ' ' << std::setw(8) << result._size << '\n';
    return false;
  case (c::PEER_SHUTDOWN):
    std::cout << "PC " << static_cast<int>(result._op) << ' ' << std::setw(8) << result._size << '\n';
    return true;
  case (c::NOT_CONNECTED):
    std::cout << "NC " << static_cast<int>(result._op) << ' ' << std::setw(8) << result._size << '\n';
    return true;
  case (c::ERROR):
    std::cout << "ER " << static_cast<int>(result._op) << ' ' << std::setw(8) << result._size << " ERROR=" << result._error << '\n';
    return true;
  }
}

int main()
{
  using namespace miniTCP;

  auto data = std::vector<char>(1024 * 1024 * 3, 'b');
  int i = 0;
  TCPServer s("192.168.178.20", 10001, true);
  while (true)
  {
    if (!s.isConnected())
    {
      auto r = s.open();
      if (r._op != OpenCode::OK)
      {
        std::cout << "Failed to open " << static_cast<int>(r._op) << ' ' << static_cast<int>(r._step) << ' ' << r._error << std::endl;
        continue;
      }
    }

    auto read = s.read(reinterpret_cast<void *>(data.data()), data.size(), true);
    if (handleError(read, true))
    {
      s.close();
      continue;
    }

    // auto wrote = s.write(reinterpret_cast<void *>(data.data()), data.size());
    // if (handleError(wrote))
    // {
    //   s.close();
    //   continue;
    // }
  }

  return 0;
}