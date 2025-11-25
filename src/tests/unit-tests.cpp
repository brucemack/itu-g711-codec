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

    float sampleRate = 8000;
    float f = 85;
    float omega = 2 * 3.14156 * f / sampleRate;
    float phi = 0;
    unsigned frameCount = 12;
    const unsigned frameLen = 80;

    for (unsigned j = 0; j < frameCount; j++) {
        int16_t frame[frameLen];
        int16_t outFrame[frameLen];
        // Put the tone into the input frame
        for (unsigned i = 0; i < frameLen; i++) {
            frame[i] = 0.5 * 32767.0f * std::cos(phi);
            phi += omega;
        }
        // Create a few erasures to demonstrate interpolation
        if (j == 4 || j == 5 || j >= 8) {
            // We do this for charting purposes only
            for (unsigned i = 0; i < frameLen; i++)
                frame[i] = 0;
            plc.badFrame(outFrame, frameLen);
        }
        // Otherwise feed good audio 
        else {
            plc.goodFrame(frame, outFrame, frameLen);
        }
        // For display
        for (unsigned i = 0; i < frameLen; i++) 
            cout << outFrame[i] << "\t" << frame[i] << endl;
    }
}

/**
 * Testing to make sure the tail of the transmission is 
 * attenuated.
 */
static void test_3() {

    Plc plc;

    float sampleRate = 8000;
    float f = 85;
    float omega = 2 * 3.14156 * f / sampleRate;
    float phi = 0;
    unsigned frameCount = 12;
    const unsigned frameLen = 80;

    for (unsigned j = 0; j < frameCount; j++) {

        int16_t inFrame[frameLen];
        int16_t outFrame[frameLen];

        // Put the tone into the input frame
        for (unsigned i = 0; i < frameLen; i++) {
            inFrame[i] = 0.5 * 32767.0f * std::cos(phi);
            phi += omega;
        }

        // Create a few erasures to demonstrate interpolation
        if (j > 4) {
            // We do this for charting purposes only
            for (unsigned i = 0; i < frameLen; i++)
                inFrame[i] = 0;
            plc.badFrame(outFrame, frameLen);
        }
        // Otherwise feed good audio 
        else {
            plc.goodFrame(inFrame, outFrame, frameLen);
        }
        // For display
        for (unsigned i = 0; i < frameLen; i++) 
            cout << outFrame[i] << "\t" << inFrame[i] << endl;
    }
}

static void test_4() {

    Plc plc;

    float sampleRate = 8000;
    unsigned frameCount = 8;
    const unsigned frameLen = 80;

    for (unsigned j = 0; j < frameCount; j++) {

        int16_t inFrame[frameLen];
        int16_t outFrame[frameLen];

        // Put the tone into the input frame
        for (unsigned i = 0; i < frameLen; i++)
            inFrame[i] = 0;

        // Create a few erasures to demonstrate interpolation
        if (j > 4) {
            plc.badFrame(outFrame, frameLen);
        }
        // Otherwise feed good audio 
        else {
            plc.goodFrame(inFrame, outFrame, frameLen);
        }
        // For display
        for (unsigned i = 0; i < frameLen; i++) 
            cout << outFrame[i] << "\t" << inFrame[i] << endl;
    }
}

int main(int,const char**) {
    //test_1();
    //test_2();
    //test_3();
    test_4();
    return 0;
}

