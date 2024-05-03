#include <vector>
#include <gtest/gtest.h>

#include "arrayProduct.h"

class ArrayProductTest : public ::testing::Test {
protected:
    void SetUp() override {
        input = {1.0, 2.0, 3.0, 4.0};
    }

    void TearDown() override {
        input.clear();
    }

    std::vector<double> input;
};

TEST_F(ArrayProductTest, MultiplierIs2) {
    double multiplier = 2.0;
    std::vector<double> expected = {2.0, 4.0, 6.0, 8.0};

    arrayProduct(input, multiplier);

    ASSERT_EQ(input, expected);
}

TEST_F(ArrayProductTest, MultiplierIs0) {
    double multiplier = 0.0;
    std::vector<double> expected = {0.0, 0.0, 0.0, 0.0};

    arrayProduct(input, multiplier);

    ASSERT_EQ(input, expected);
}

TEST_F(ArrayProductTest, MultiplierIsNegative) {
    double multiplier = -1.5;
    std::vector<double> expected = {-1.5, -3.0, -4.5, -6.0};

    arrayProduct(input, multiplier);

    ASSERT_EQ(input, expected);
}
