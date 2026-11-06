
#include<string>
#include <iostream>
#include "helper.hpp"
void exitError(const char* Error) {
    std::cout << "[\x1b[93mFATAL ERROR\x1b[0m]" << Error << std::endl;
    exit(EXIT_FAILURE);
}
