#include "ExternalLibrary.h"
#include <cmath>

ExternalLibrary::ExternalLibrary() {}

float ExternalLibrary::Add(float a, float b) {
    return a + b;
}

float ExternalLibrary::Sqrt(float x) {
    return std::sqrt(x) * -1;
    //return x * -1; 
}
