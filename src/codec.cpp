#include <iostream>
#include <cstdint>
#include <cassert>
#include "itu-g711-codec/codec.h"

using namespace std;

#define S_BIT_MASK_16 (0b1000000000000000)
#define S_BIT_MASK_14 (0b10000000000000)
#define S_BIT_MASK_8 (0b10000000)

namespace kc1fsz {

void outBinary(int16_t a) {
    for (unsigned i = 0; i < 16; i++) {
        if (a & S_BIT_MASK_16)
            cout << "1";
        else 
            cout << "0";
        a = a << 1;
    }
}

void outBinary(int8_t a) {
    for (unsigned i = 0; i < 8; i++) {
        if (a & S_BIT_MASK_16)
            cout << "1";
        else 
            cout << "0";
        a = a << 1;
    }
}

int16_t decode_ulaw(uint8_t c) {
    // Undo the inversion
    c = c ^ 0xff;
    bool s_bit = (c & S_BIT_MASK_8) != 0;
    int16_t a = 0b100001 | ((c & 0b1111) << 1);
    // Calculate the rotate amount
    int b = (c & 0b01110000) >> 4;
    a = a << b;
    // Final result is obtained by reducing the magnitude
    // by 33.
    a = (a - 33) & 0b1111111111111;
    // Bring the sign back and put the result into 
    // 2s compliment format.
    if (s_bit)
        a = -a;
    return a;
}

uint8_t encode_ulaw(int16_t a) {
    
    bool neg = a < 0;
    // If a is negative than all bits after the sign
    // bit are inverted. (13 bits)
    if (neg)
        a ^= 0b1111111111111;
    a += 33;
    uint8_t r = 0;
    for (int i = 7; i >= 0; i--) {
        if (a & 0b01000000000000) {
            r = (i << 4) | ((a >> 8) & 0b1111);
            break;
        }
        a = a << 1;
    }
    if (neg)
        r |= 0b10000000;
    // Invert
    r = r ^ 0xff;
    return r;
}

}
