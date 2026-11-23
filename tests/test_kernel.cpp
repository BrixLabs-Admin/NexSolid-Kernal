#include "nexsolid/Kernel.hpp"

#include <cmath>
#include <exception>
#include <iostream>

int main() {
    try {
        const auto box = nexsolid::makeBox(10.0, 20.0, 30.0);
        if (box.IsNull()) {
            std::cerr << "makeBox returned a null shape\n";
            return 1;
        }
        if (!nexsolid::isValid(box)) {
            std::cerr << "makeBox returned an invalid shape\n";
            return 1;
        }
        if (std::abs(nexsolid::volume(box) - 6000.0) > 1e-7) {
            std::cerr << "box volume did not match 6000\n";
            return 1;
        }

        bool rejected_zero_dimension = false;
        try {
            (void)nexsolid::makeBox(1.0, 0.0, 1.0);
        } catch (const std::exception&) {
            rejected_zero_dimension = true;
        }
        if (!rejected_zero_dimension) {
            std::cerr << "makeBox accepted a zero dimension\n";
            return 1;
        }

        std::cout << "Kernel smoke tests passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Unexpected test exception: " << error.what() << '\n';
        return 1;
    }
}
