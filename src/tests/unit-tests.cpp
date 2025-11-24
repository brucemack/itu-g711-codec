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

    // Fill the pitch buffer with test data
    float sampleRate = 8000;
    float f = 165;
    float f2 = 100;
    float omega = 2 * 3.14156 * f / sampleRate;
    float omega2 = 2 * 3.14156 * f2 / sampleRate;
    float phi = 0;
    float phi2 = 0;
    unsigned frameCount = 7;
    const unsigned frameLen = 80;
    int16_t frame[frameLen];
    int16_t outFrame[frameLen];

    for (unsigned j = 0; j < frameCount; j++) {
        for (unsigned i = 0; i < frameLen; i++) {
            frame[i] = 0.5 * 32767.0f * std::cos(phi) + 0.0 * 32767.0f * std::cos(phi2);
            phi += omega;
            phi2 += omega2;
        }
        if (j == 4 || j == 5) {
            plc.badFrame(outFrame, frameLen);
        }
        else {
            plc.goodFrame(frame, outFrame, frameLen);
        }
        // Debug (after the first)
        if (j > 0) {
            for (unsigned i = 0; i < frameLen; i++) 
                cout << outFrame[i] << endl;
        }
    }
}

int main(int,const char**) {
    test_1();
    test_2();
    return 0;
}

