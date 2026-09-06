#include <cmath>
#include <iostream>
#include <numbers>
int main() {
    const double diagonal = std::hypot(6000.0, 4000.0);
    std::cout << "Fixture environment-v1: diagonal=" << diagonal << " mm\n";
    return std::abs(diagonal - 7211.102550927979) < 1e-9 && std::numbers::pi > 3.14 ? 0 : 1;
}
