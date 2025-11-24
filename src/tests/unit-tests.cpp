#include <iostream>
#include <cstdint>
#include <cassert>
#include <cmath>

#include "itu-g711-codec/codec.h"

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

    const float sampleRate = 8000;
    const float secondsPerSample = 1.0 / sampleRate;
    // A 10ms block at 8kHz
    const unsigned blockSize = 80;
    // The period of a 66 Hz pitch
    const unsigned pitchPeriodMax = 120; 
    // The period of a 200 Hz pitch
    const unsigned pitchPeriodMin = 40; 
    // The length of the correlation period used when searching for the pitch
    const unsigned corrLen = 160;
    const float minPower = 250;
    // The number of taps at which the correlation will be computed when 
    // looking for best fit.
    const unsigned taps = pitchPeriodMax - pitchPeriodMin;
    const unsigned pitchBufferLen = 3 * pitchPeriodMax;
    int16_t pitchBuffer[pitchBufferLen];

    // Fill the pitch buffer with test data
    float f = 165;
    float f2 = 100;
    float omega = 2 * 3.14156 * f / sampleRate;
    float omega2 = 2 * 3.14156 * f2 / sampleRate;
    float phi = 0;
    float phi2 = 0;
    for (unsigned i = 0; i < pitchBufferLen; i++) {
        pitchBuffer[i] = 0.5 * 32767.0f * std::cos(phi) + 0.0 * 32767.0f * std::cos(phi2);
        phi += omega;
        phi2 += omega2;
    }

    // Setup the anchor points for the correlation. p1 is the beginning
    // of the newest 20ms block in the pitch buffer.
    const unsigned p1 = pitchBufferLen - corrLen; 

    float energy = 0;
    float corr = 0;
    float bestCorr = 0;
    unsigned bestOffset = 0;
    unsigned tapOffsetLow = pitchPeriodMin;
    unsigned tapOffsetHigh = pitchPeriodMax;
    unsigned step = 2;

    // During the coarse search we test every other tap. The scan starts
    // from the longest pitch period and ends at the highest pitch period.
    for (unsigned tapOffset = tapOffsetHigh; tapOffset >= tapOffsetLow; 
        tapOffset -= step) {
        energy = 0;
        corr = 0;
        unsigned p0 = pitchBufferLen - corrLen - tapOffset; 
        for (unsigned i = 0; i < corrLen; i += step) {
            int16_t s0 = pitchBuffer[p0 + i];
            int16_t s1 = pitchBuffer[p1 + i];
            energy += (float)s0 * (float)s0;
            corr += (float)s0 * (float)s1;
        }
        float scale = std::max(energy, minPower);
        corr = abs(corr / sqrt(scale));
        //float tapSeconds = secondsPerSample *(float)tapOffset;
        //float tapFreq = 1.0 / tapSeconds;
        //cout << tapFreq << " " << corr << endl;
        // Any better?
        if (corr >= bestCorr) {
            bestCorr = corr;
            bestOffset = tapOffset;
        }
    }

    // Fine tuning does exactly the same thing, but just focuses on three taps 
    // around the best match from the coarse search.
    tapOffsetLow = std::max((int)bestOffset - 1, (int)pitchPeriodMin);
    tapOffsetHigh = std::min((int)bestOffset + 1, (int)pitchPeriodMax);
    step = 1;
    // We start from scratch since the step size is different
    bestOffset = 0;
    bestCorr = 0;

    for (unsigned tapOffset = tapOffsetHigh; tapOffset >= tapOffsetLow; 
        tapOffset -= step) {
        energy = 0;
        corr = 0;
        unsigned p0 = pitchBufferLen - corrLen - tapOffset; 
        for (unsigned i = 0; i < corrLen; i += step) {
            int16_t s0 = pitchBuffer[p0 + i];
            int16_t s1 = pitchBuffer[p1 + i];
            energy += (float)s0 * (float)s0;
            corr += (float)s0 * (float)s1;
        }
        float scale = std::max(energy, minPower);
        corr = abs(corr / sqrt(scale));
        if (corr >= bestCorr) {
            bestCorr = corr;
            bestOffset = tapOffset;
        }
    }

    // Convert offset to frequency
    float bestSeconds = secondsPerSample * (float)bestOffset;
    cout << "bestOffset " << bestOffset << endl;
    cout << "bestFreq " << 1.0 / bestSeconds << endl;
    cout << "bestCorr " << bestCorr << endl;

}

int main(int,const char**) {
    test_1();
    test_2();
    return 0;
}

