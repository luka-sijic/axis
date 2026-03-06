#pragma once
#include <unordered_map>
#include <functional>

namespace axis {
struct req {
    std::string method;
    std::string path;
};

struct req_hash {
  std::size_t operator()(const req& r) const noexcept {
    std::size_t h1 = std::hash<std::string_view>{}(r.method);
    std::size_t h2 = std::hash<std::string_view>{}(r.path);
    return h1 ^ (h2 << 1);
  }
};

struct req_equal {
  bool operator()(const req& a, const req& b) const noexcept {
    return a.method == b.method && a.path == b.path;
  }
};

using Handler = std::string(*)();

class Router {
public:

    static std::string handle_get() {
      return "hello friend";
    }
    static std::string handle_post() {
      return "wow nice post req";
    }
    Router() {
      table_[{"GET", "/"}] = &Router::handle_get;
      table_[{"POST", "/"}] = &Router::handle_post;
    }
    Handler get_handler(const std::string& method, const std::string& path) {
        return table_[{method, path}];
    }
private:
  std::unordered_map<req, Handler, req_hash, req_equal> table_;
};
};
