#include <arpa/inet.h>
#include <cerrno>
#include <future>
#include <thread>
#include <chrono>
#include <fcntl.h>
#include <iostream>
#include <liburing.h>
#include <axis/parser.hpp>
#include <span>
#include <sys/socket.h>
#include <unistd.h>
#include <axis/router.hpp>
/*
auto ln = axis::TCP::listener();
*/

namespace axis {
struct endpoint {
  int ip;
  uint16_t port;
};
enum : std::uint64_t {
  RECV_OP = 1,
  SEND_OP = 2,
};

class tcp {
public:
  struct Stream {
    int fd_{-1};
    Router router_{};
    io_uring ring_{};

    std::array<std::byte, 4096> rx_{};
    std::string tx_;

    Stream(int fd) : fd_(fd) {
      io_uring_queue_init(1024, &ring_, 0);
      io_uring_submit(&ring_);
    }

    void process() {
      recv();
      for (;;) {
        io_uring_cqe *cqe{};
        int rc = io_uring_wait_cqe(&ring_, &cqe);
        if (rc < 0)
          continue;
        int res = cqe->res;
        switch (cqe->user_data) {
        case RECV_OP: {
          std::cerr << "We recv\n";
          std::string_view sv(reinterpret_cast<const char *>(rx_.data()),
                              static_cast<size_t>(res));
          std::cerr << sv << "\n";
          auto fut = std::async(std::launch::async, [&] {
                std::this_thread::sleep_for(std::chrono::seconds(5));
                std::cerr << "async work done";
                                });
          http_req req;
          axis::parser::parse(sv, req);
          auto fn = router_.get_handler(req.method, req.path);
          auto str = fn();
          std::cerr << str << "\n";
          send(axis::parser::build_response(str));
          recv();
          break;
        }
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
      // return ::send(fd_, bytes.data(), bytes.size_bytes(), 0);
    }

    ssize_t recv() {
      if (io_uring_sqe *sqe = io_uring_get_sqe(&ring_)) {
        io_uring_prep_recv(sqe, fd_, rx_.data(), rx_.size(), 0);
        sqe->user_data = RECV_OP;

        io_uring_submit(&ring_);

        return 0;
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

  explicit tcp(uint16_t port) { listener(port); }

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
      // std::cerr << "bind failed: " << std::strerror(errno) << std::endl;
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
