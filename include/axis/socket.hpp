#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
/*
auto ln = axis::TCP::listener();
*/

namespace axis {
class tcp {
public:
  struct Stream {
    int fd{-1};

    size_t send(std::span<const std::byte> bytes) {
      return ::send(fd, bytes.data(), bytes.size_bytes(), 0);
    }

    size_t recv(std::span<std::byte> bytes) {
      return ::recv(fd, bytes.data(), bytes.size_bytes(), 0);
    }

    size_t send(std::string_view s) {
      auto bytes = std::as_bytes(std::span{s.data(), s.size()});
      return send(bytes);
    }

    void print(std::span<std::byte> bytes) {
      std::cerr << std::string_view(
          reinterpret_cast<const char *>(bytes.data()));
    }
  };

  void listener(uint16_t port) {
    fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in server_addr{
        .sin_family = AF_INET,
        .sin_port = htons(port),
        .sin_addr = {.s_addr = htonl(INADDR_ANY)},
    };

    int one = 1;
    ::setsockopt(fd_, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(one));

    if (::bind(fd_, reinterpret_cast<sockaddr *>(&server_addr),
               sizeof(server_addr)) < 0) {
      std::cerr << "bind failed: " << std::strerror(errno) << std::endl;
      ::close(fd_);
      return;
    }

    listen(fd_, SOMAXCONN);
  }

  Stream accept() {
    for (;;) {
      int cfd = ::accept(fd_, nullptr, nullptr);
      if (cfd >= 0)
        return Stream{.fd = cfd};
    }
  }

private:
  int fd_{};
};
}; // namespace axis
