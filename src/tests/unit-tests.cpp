#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

#include "itu-g711-codec/codec.h"
#include "itu-g711-plc/Plc.h"

using namespace std;
using namespace kc1fsz;

/*
These test cases are drawn from the official standards document. See:

https://www.itu.int/rec/dologin_pub.asp?lang=e&id=T-REC-G.711-198811-I!!PDF-E&type=items

Tables 2a and 2b are relevant.

The <<2 is needed because the test cases are all stated in terms of 14-bit PCM 
values whereas the functions assume 16-bit PCM values.

16-bit PCM values should range from -32,767 to +32,767
14-bit PCM values should range from -8,192 to +8,192
*/

void test_1() {

    assert(encode_ulaw(-8160 << 2) == 0b00000000);
    assert(encode_ulaw(-8159 << 2) == 0b00000000);
    assert(encode_ulaw(-8158 << 2) == 0b00000000);
    assert(encode_ulaw(-96 << 2)   == 0b01011111);
    assert(encode_ulaw(-1 << 2)    == 0b01111111);
    assert(encode_ulaw(0 << 2)     == 0b11111111);
    assert(encode_ulaw(1 << 2)     == 0b11111110);
    assert(encode_ulaw(8031 << 2)  == 0b10000000);
    assert(encode_ulaw(8158 << 2)  == 0b10000000);
    // Beyond the range?
    assert(encode_ulaw(8159 << 2)  == 0b10000000);
    assert(encode_ulaw(8160 << 2)  == 0b10000000);

    assert(decode_ulaw(0b10000000) == 8031 << 2);
    assert(decode_ulaw(0b01111111) == 0 << 2);
    assert(decode_ulaw(0b01111110) == -2 << 2);
    assert(decode_ulaw(0b01101111) == -33 << 2);
    assert(decode_ulaw(0b01011111) == -99 << 2);
    assert(decode_ulaw(0b00000001) == -7775 << 2);
    assert(decode_ulaw(0b00000000) == -8031 << 2);
}

void test_2() {
    Plc plc;
    plc.test();
}

int main(int,const char**) {
    test_1();
    test_2();
    return 0;
}

