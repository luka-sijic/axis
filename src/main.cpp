#include <axis/sbbf.hpp>
#include <axis/socket.hpp>
#include <axis/spsc.hpp>
#include <iostream>

/*
cmake -B build -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build --target axis_test
./build/axis_test
*/

int main() {
  auto q = axis::spsc<uint32_t, 64>();
  q.try_push(10);
  q.try_push(20);
  uint32_t h;
  q.try_pop(h);
  std::cout << "HELLO: " << h << std::endl;

  auto bf = axis::sbbf(10'000'000, 0.01);
  bf.insert("hi");
  std::cout << bf.possiblyContains("hi") << "\n";
  std::cout << bf.possiblyContains("wow") << "\n";

  axis::tcp a;
  a.listener(8080);
  auto s = a.accept();
  std::array<std::byte, 4096> storage{};
  s.recv(std::span<std::byte>(storage));
  s.send("hi");
  std::cout << std::string_view(reinterpret_cast<const char *>(storage.data()))
            << std::endl;
  return 0;
}
