/** Copyright (C) 2023  nolleh (nolleh7707@gmail.com) **/

// https://opensource.com/article/22/1/unit-testing-googletest-ctest
#include <gtest/gtest.h>

TEST(MessageTest, Test) {
  int a = 0;
  int b = 0;

  EXPECT_EQ(a, b);

  a = 1;

  EXPECT_NE(a, b);
}
