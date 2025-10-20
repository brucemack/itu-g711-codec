#include <iostream>
#include <cstdint>
#include <cassert>
#include "itu-g711-codec/codec.h"

using namespace std;
using namespace kc1fsz;

int main(int,const char**) {

    assert(encode_ulaw(-8159) == 0b00000000);
    assert(encode_ulaw(-8158) == 0b00000000);
    assert(encode_ulaw(-96)   == 0b01011111);
    assert(encode_ulaw(-1)    == 0b01111111);
    assert(encode_ulaw(0)     == 0b11111111);
    assert(encode_ulaw(1)     == 0b11111110);
    assert(encode_ulaw(8031)  == 0b10000000);
    assert(encode_ulaw(8158)  == 0b10000000);

    assert(decode_ulaw(0b10000000) == 8031);
    assert(decode_ulaw(0b01111111) == 0);
    assert(decode_ulaw(0b01111110) == -2);
    assert(decode_ulaw(0b01101111) == -33);
    assert(decode_ulaw(0b01011111) == -99);
    assert(decode_ulaw(0b00000001) == -7775);
    assert(decode_ulaw(0b00000000) == -8031);

    return 0;
}
