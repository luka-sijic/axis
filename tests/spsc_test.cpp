#include <atomic>
#include <axis/spsc.hpp>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

class SPSCTest : public ::testing::Test {
protected:
  axis::spsc<int, 1024> queue;
};

TEST_F(SPSCTest, InitiallyEmpty) {
  EXPECT_TRUE(queue.empty());
  EXPECT_EQ(0, queue.size());
}

TEST_F(SPSCTest, PushPopSingleElement) {
  EXPECT_TRUE(queue.try_push(10));

  int val = 0;
  EXPECT_TRUE(queue.try_pop(val));
  EXPECT_EQ(val, 10);
  EXPECT_TRUE(queue.empty());
}
