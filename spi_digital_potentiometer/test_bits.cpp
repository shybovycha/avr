#include <iostream>
#include <vector>
#include <string>

int setBit(int n, int b) {
    return n | (1 << b);
}

int clearBit(int n, int b) {
    return n & (~(1 << b));
}

std::string int2bin(int n) {
    std::string s;

    for (int i = 0; i < 8; ++i) {
        if (n & (1 << 7)) {
            s += "1";
        } else {
            s += "0";
        }

        // s += std::to_string((n & (1 << 7)) >> 7);
        n <<= 1;
    }

    return s;
}

int main() {
    unsigned char n = 255;

    for (int i = 0; i < 8; ++i) {
        std::cout << "n = " << int2bin(n) << " ";
        std::cout << "MSB = " << ((n & (1 << 7)) ? "1" : "0") << std::endl;

        n <<= 1;
    }
    // std::cout << "setBit(n, 0) = " << int2bin(setBit(n, 0)) << std::endl;
    // std::cout << "setBit(n, 1) = " << int2bin(setBit(n, 1)) << std::endl;
    // std::cout << "clearBit(n, 0) = " << int2bin(clearBit(n, 0)) << std::endl;
    // std::cout << "clearBit(n, 1) = " << int2bin(clearBit(n, 1)) << std::endl;

    return 0;
}

