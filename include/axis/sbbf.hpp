#pragma once
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>

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
    size_t m_bits = static_cast<size_t>(std::ceil(m_bits_f));
    if (m_bits == 0)
      m_bits = 64;

    size_t blocks = (m_bits + (BITS_PER_BLOCK - 1)) / BITS_PER_BLOCK;
    if (blocks == 0)
      blocks = 1;
    capacity_ = round_up_pow2(blocks);

    filter_ = std::make_unique<Block[]>(capacity_);
  }

  void add(std::string_view key) noexcept {
    const uint64_t h = hash64(key);
    const uint64_t h1 = mix64(h);
    const uint64_t h2 = mix64(h1) | 1ULL;

    const size_t idx = static_cast<size_t>(h1) & (capacity_ - 1);
    const Block mask = make_mask_block(h1, h2);
    or_block(filter_[idx], mask);
  }

  void insert(std::string_view key) noexcept { add(key); }

  [[nodiscard]] bool possiblyContains(std::string_view key) const noexcept {
    uint64_t h = hash64(key);
    uint64_t h1 = mix64(h);
    uint64_t h2 = mix64(h1) | 1ULL;

    const size_t idx = static_cast<size_t>(h1) & (capacity_ - 1);

    const Block mask = make_mask_block(h1, h2);
    return simd_block_contains(filter_[idx], mask);
  }

  [[nodiscard]] bool possibly_contains(std::string_view key) const noexcept {
    return possiblyContains(key);
  }

  void clear() noexcept { std::fill_n(filter_.get(), capacity_, Block{}); }

  [[nodiscard]] size_t block_count() const noexcept { return capacity_; }

private:
  static uint64_t hash64(std::string_view key) noexcept {
    uint64_t h = 14695981039346656037ULL; // FNV offset basis
    for (unsigned char c : key) {
      h ^= static_cast<uint64_t>(c);
      h *= 1099511628211ULL; // FNV prime
    }
    h ^= static_cast<uint64_t>(key.size());
    return mix64(h);
  }

  static uint64_t mix64(uint64_t x) noexcept {
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdULL;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ULL;
    x ^= x >> 33;
    return x;
  }

  static Block make_mask_block(uint64_t h1, uint64_t h2) noexcept {
    const uint32_t x = static_cast<uint32_t>((h1 >> 32) ^ h2);

    Block m{};
    for (size_t i = 0; i < LANES; ++i) {
      uint32_t bit = (x * SALT[i]) >> 27;
      m.w[i] = (1u << bit);
    }
    return m;
  }

  static void or_block(Block &dst, const Block &mask) noexcept {
    for (size_t i = 0; i < LANES; ++i) {
      dst.w[i] |= mask.w[i];
    }
  }

  static bool simd_block_contains(const Block &block,
                                  const Block &mask) noexcept {
#if defined(SBBF_HAS_AVX2)
    const __m256i block_vec =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(block.w));
    const __m256i mask_vec =
        _mm256_loadu_si256(reinterpret_cast<const __m256i *>(mask.w));
    const __m256i masked = _mm256_and_si256(block_vec, mask_vec);
    const __m256i cmp = _mm256_cmpeq_epi32(masked, mask_vec);
    return _mm256_movemask_ps(_mm256_castsi256_ps(cmp)) == 0xFF;
#elif defined(SBBF_HAS_SSE2)
    for (size_t i = 0; i < LANES; i += 4) {
      const __m128i block_vec =
          _mm_loadu_si128(reinterpret_cast<const __m128i *>(&block.w[i]));
      const __m128i mask_vec =
          _mm_loadu_si128(reinterpret_cast<const __m128i *>(&mask.w[i]));
      const __m128i masked = _mm_and_si128(block_vec, mask_vec);
      const __m128i cmp = _mm_cmpeq_epi32(masked, mask_vec);
      if (_mm_movemask_ps(_mm_castsi128_ps(cmp)) != 0xF) {
        return false;
      }
    }
    return true;
#else
    for (size_t i = 0; i < LANES; ++i) {
      if ((block.w[i] & mask.w[i]) != mask.w[i]) {
        return false;
      }
    }
    return true;
#endif
  }

  static size_t round_up_pow2(size_t x) noexcept {
    if (x <= 1)
      return 1;
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
  std::unique_ptr<Block[]> filter_;
};
} // namespace axis
