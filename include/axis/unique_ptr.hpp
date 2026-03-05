#include <iostream>

/*
unique_ptr<T> a
- exclusive ownership
- automatic cleanup and prevents double deletion
- no copy semantics
*/

namespace axis {
template <typename T> class unique_ptr {
public:
  unique_ptr() : data_(new T) {}
  ~unique_ptr() { delete data_; }

  unique_ptr(const unique_ptr &) = delete;
  unique_ptr &operator=(const unique_ptr &) = delete;
  unique_ptr(unique_ptr &&) = default;
  unique_ptr &operator=(unique_ptr &&) = default;

private:
  T *data_{};
};
} // namespace axis
