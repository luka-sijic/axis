#pragma once
#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <string_view>
#include <vector>

// SIMD backends
#if defined(__AVX2__)
#include <immintrin.h>
#define SBBF_HAS_AVX2 1
#elif defined(__SSE2__)
#include <emmintrin.h>
#define SBBF_HAS_SSE2 1
#elif defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
#include <arm_neon.h>
#define SBBF_HAS_NEON 1
#endif

namespace axis {
class sbbf {
  static constexpr size_t LANES = 8;
  static constexpr size_t BITS_PER_BLOCK = LANES * 32; // 256
  // 4x uint64_t = 256 bits; AVX2-friendly on x86, still fine on ARM
  struct alignas(32) Block {
    uint32_t w[LANES];
  };
  static constexpr uint32_t SALT[LANES] = {
      0x47b6137bu, 0x44974d91u, 0x8824ad5bu, 0xa2b7289du,
      0x705495c7u, 0x2df1424bu, 0x9efc4947u, 0x5c6bfb31u};

public:
  explicit sbbf(size_t n, double p) {
    if (n == 0)
      n = 1;
    if (!(p > 0.0 && p < 1.0))
      p = 0.01;

    const double ln2 = std::log(2.0);

    const double m_bits_f =
        (-static_cast<double>(n) * std::log(p)) / (ln2 * ln2);
    size_t m_bits = static_cast<size_t>(ceil(m_bits_f));
    if (m_bits == 0)
      m_bits = 64;

    size_t blocks = (m_bits + (BITS_PER_BLOCK - 1)) / BITS_PER_BLOCK;
    if (blocks == 0)
      blocks = 1;
    capacity_ = round_up_pow2(blocks);

    // Allocate filter
    filter_.assign(capacity_, Block{});
  }

  bool possiblyContains(const std::string_view &key) const noexcept {
    uint64_t h = hash64(key);
    uint64_t h1 = mix64(h);
    uint64_t h2 = mix64(h1) | 1ULL;

    const size_t idx = static_cast<size_t>(h1) & (capacity_ - 1);

    Block mask = make_mask_block(h1, h2);
    return simd_block_contains(filter_[idx], mask);
  }

private:
  Block make_block_mask(uint64_t h1, int64_t h2) const noexcept {
    uint32_t x = static_cast<uint32_t>(h2);

    Block m{};
    for (size_t i = 0; i < LANES; ++i) {
      uint32_t bit = (x * SALT[i]) >> 27;
      m.w[i] = (1u << bit);
    }
    return m;
  }

  static size_t round_up_pow2(size_t x) noexcept {
    --x;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    if constexpr (sizeof(size_t) == 8)
      x |= x >> 32;
    return x + 1;
  }
  size_t capacity_;
  std::vector<Block> filter_;
};
} // namespace axis
