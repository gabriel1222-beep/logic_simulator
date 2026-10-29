
#include<string>
#include <iostream>
#include "helper.hpp"
void exitError(std::string Error) {
    std::cout << "[\x1b[93mFATAL ERROR\x1b[0m]" << Error << std::endl;
    exit(EXIT_FAILURE);
}
