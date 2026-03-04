#include <arpa/inet.h>
#include <cerrno>
#include <iostream>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
#include <liburing.h>
/*
auto ln = axis::TCP::listener();
*/

namespace axis {
enum : std::uint64_t {
  RECV_OP = 1,
  SEND_OP = 2,
};

class tcp {
public:
  struct Stream {
    int fd_{-1};
    io_uring ring_{};

    Stream(int fd) : fd_(fd) {
      io_uring_queue_init(1024, &ring_, 0);
      io_uring_submit(&ring_);
    }

    void process() {
      for (;;) {
        io_uring_cqe *cqe{};
        int rc = io_uring_wait_cqe(&ring_, &cqe);
        switch (cqe->user_data) {
          case RECV_OP:
            std::cerr << "We recv\n";
            break;
          case SEND_OP:
            std::cerr << "We send\n";
            break;
        }
        io_uring_cqe_seen(&ring_, cqe);
      }
    }

    size_t send(std::span<const std::byte> bytes) {
      if (io_uring_sqe *sqe = io_uring_get_sqe(&ring_)) {
        io_uring_prep_send(sqe, fd_, bytes.data(), bytes.size_bytes(), 0);
        sqe->user_data = SEND_OP;
        io_uring_submit(&ring_);
      }
      return -1;
      //return ::send(fd_, bytes.data(), bytes.size_bytes(), 0);
    }

    ssize_t recv(std::span<std::byte> bytes) {
      if (io_uring_sqe *sqe = io_uring_get_sqe(&ring_)) {
        io_uring_prep_recv(sqe, fd_, bytes.data(), bytes.size_bytes(), 0);
        sqe->user_data = RECV_OP;
        
        io_uring_submit(&ring_);

        io_uring_cqe* cqe = nullptr;
        io_uring_wait_cqe(&ring_, &cqe);

        ssize_t res = cqe->res;

        io_uring_cqe_seen(&ring_, cqe);
        return res;
      }
      return -1;
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
      //std::cerr << "bind failed: " << std::strerror(errno) << std::endl;
      ::close(fd_);
      return;
    }

    listen(fd_, SOMAXCONN);
  }

  Stream accept() {
    for (;;) {
      int cfd = ::accept(fd_, nullptr, nullptr);
      if (cfd >= 0)
        return Stream(cfd);
    }
  }

private:
  int fd_{};
};
}; // namespace axis
