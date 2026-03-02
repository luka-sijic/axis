- Running test instructions
- cmake -B build -DBUILD_TESTING=ON -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
- cmake --build build
- ./build/tests/axis_tests

# SPSC (Single Producer Single Consumer queue)
- spsc<T, size_t N> [N must be power of two]
- bool try_push(T)
- bool try_pop(T)
- size_t size()
- bool empty()

# SBBF (Split Block Bloom Filter)
- sbbf(size_t n, double p) [n = expected amount of elements, p = tolerated false positive rate]
- bool insert(string)
- bool possiblyContains(string)
- void clear()
- size_t block_count()

# Socket
- tcp::listener(int port)
- 
