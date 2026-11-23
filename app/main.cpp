#include "nexsolid/Kernel.hpp"

#include <iostream>

int main() {
    const auto box = nexsolid::makeBox(10.0, 20.0, 30.0);
    std::cout << "NexSolid Kernel starter: valid box, volume = "
              << nexsolid::volume(box) << '\n';
    return nexsolid::isValid(box) ? 0 : 1;
}
