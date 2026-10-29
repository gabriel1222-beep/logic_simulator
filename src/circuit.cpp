#include "circuit.hpp"

#include <iostream>

#include <fmt/format.h>

void Circuit::CacheLinesNeeded(long double bytes) {
    std::cout << fmt::format(
        "{} bytes needs {} cache lines\n",
        bytes,
        bytes / 64
    );
}

