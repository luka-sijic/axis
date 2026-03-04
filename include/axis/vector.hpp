#include <iostream>

namespace axis {
template <typename T> class vector {
public:
  vector() : capacity_(0), items_(std::make_unique<T[]>(0)) {}
  vector(size_t capacity)
      : capacity_(capacity), items_(std::make_unique<T[]>(capacity_)) {}

  T at(size_t idx) {
    if (idx < capacity_) {
      return items_[idx];
    }
    return -1;
  }

  T &operator[](size_t idx) { return items_[idx]; }

private:
  size_t capacity_;
  std::unique_ptr<T[]> items_;
};
}; // namespace axis
