#include "StaticTest.h"
#include <cmath>

StaticTest::StaticTest() {}

float StaticTest::Add(float a, float b) {
    return a + b;
}

float StaticTest::Sqrt(float x) {
    return std::sqrt(x);
}
