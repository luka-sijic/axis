#pragma once
#include <atomic>
#include <memory>

/*
API
axis::spsc<std::string, 128>
try_push(T) - owns write
try_pop(T) - owns read
*/

namespace axis {
template <typename T, size_t N> class spsc {
  static_assert(N && ((N & (N - 1)) == 0), "N must be a power of two");

public:
  explicit spsc() : items_{std::make_unique<T[]>(N)} {}

  void operator=(const spsc &) = delete;
  spsc(const spsc &) = delete;
  void operator=(spsc &&) = delete;
  spsc(spsc &&) = delete;

  bool try_push(const T &item) noexcept {
    const auto readIdx = readIdx_.load(std::memory_order_acquire);
    const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

    const auto next = (writeIdx + 1) & (capacity_ - 1);
    const auto isFull = next == readIdx;

    if (isFull)
      return false;

    items_[writeIdx] = std::move(item);
    writeIdx_.store(next, std::memory_order_release);
    return true;
  }

  bool try_pop(T &item) noexcept {
    auto readIdx = readIdx_.load(std::memory_order_relaxed);
    auto writeIdx = writeIdx_.load(std::memory_order_acquire);

    if (readIdx == writeIdx)
      return false;

    item = std::move(items_[readIdx]);
    readIdx_.store((readIdx + 1) & (capacity_ - 1), std::memory_order_release);
    return true;
  }

  [[nodiscard]] size_t size() const {
    const auto readIdx = readIdx_.load(std::memory_order_relaxed);
    const auto writeIdx = writeIdx_.load(std::memory_order_relaxed);

    return (writeIdx - readIdx) & (capacity_ - 1);
  }

  [[nodiscard]] bool empty() const noexcept {
    return readIdx_.load(std::memory_order_acquire) ==
           writeIdx_.load(std::memory_order_acquire);
  }

private:
  alignas(64) std::atomic<size_t> readIdx_{};
  alignas(64) std::atomic<size_t> writeIdx_{};
  static constexpr size_t capacity_ = N;
  std::unique_ptr<T[]> items_;
};
} // namespace axis
