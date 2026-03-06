#pragma once
#include <string>

struct http_req {
  std::string method;
  std::string path;
  std::string version;
};

namespace axis {
class parser {
public:
    static std::string build_response(const std::string& body) {
        std::string res;
        
        res += "HTTP/1.1 200 OK\r\n";
        res += "Content-Length: ";
        res += std::to_string(body.size());
        res += "\r\n";
        res += "Connection: keep-alive\r\n";
        res += "\r\n";
        res += body;
        
        return res;
    }
    static bool parse(std::string_view req, http_req& res) {
        auto sp1 = req.find(' ');
        if (sp1 == std::string_view::npos) return false;

        auto sp2 = req.find(' ', sp1 + 1);
        if (sp2 == std::string_view::npos) return false;

        auto crlf = req.find("\r\n", sp2 + 1);
        if (crlf == std::string_view::npos) return false;

        res.method  = req.substr(0, sp1);
        res.path    = req.substr(sp1 + 1, sp2 - (sp1 + 1));
        res.version = req.substr(sp2 + 1, crlf - (sp2 + 1));
        return true;        
    }
private:
};
};
