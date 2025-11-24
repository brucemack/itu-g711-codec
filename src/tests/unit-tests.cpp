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
    const unsigned blockSize = 160;
    // The period of a 66 Hz pitch
    const unsigned pitchPeriodMax = 120; 
    // The period of a 200 Hz pitch
    const unsigned pitchPeriodMin = 40; 
    // The length of the correlation 
    const unsigned corrLen = 160;
    const float minPower = 250;
    // The number of taps at which the correlation will be computed when 
    // looking for best fit.
    const unsigned taps = pitchPeriodMax - pitchPeriodMin;

    int16_t pitchBuffer[3 * pitchPeriodMax];
    const int16_t* pitchBufferEnd = pitchBuffer + 3 * pitchPeriodMax;

    // Fill the pitch buffer with test data
    float f = 66;
    float f2 = 100;
    float omega = 2 * 3.14156 * f / sampleRate;
    float omega2 = 2 * 3.14156 * f2 / sampleRate;
    float phi = 0;
    float phi2 = 0;
    for (unsigned i = 0; i < 3 * pitchPeriodMax; i++) {
        pitchBuffer[i] = 0.5 * 32767.0f * std::cos(phi) + 0.1 * 32767.0f * std::cos(phi2);
        phi += omega;
        phi2 += omega2;
    }

    // Setup the anchor points for the correlation
    const int16_t* p0 = pitchBufferEnd - blockSize - pitchPeriodMax;
    const int16_t* p1 = pitchBufferEnd - blockSize;

    float energy = 0;
    float corr = 0;
    float bestCorr = 0;
    unsigned bestOffset = 0;

    // During the coarse search we test half of the taps.
    for (unsigned tapOffset = 0; tapOffset <= taps; tapOffset += 2) {
        energy = 0;
        corr = 0;
        for (unsigned i = 0; i < corrLen; i += 2) {
            int16_t s0 = p0[tapOffset + i];
            int16_t s1 = p1[i];
            energy += (float)s0 * (float)s0;
            corr += (float)s0 * (float)s1;
        }
        float scale = std::max(energy, minPower);
        corr /= sqrt(scale);
        if (corr >= bestCorr) {
            bestCorr = corr;
            bestOffset = tapOffset;
        }
    }

    cout << "bestOffset " << bestOffset << endl;
    // Convert offset to frequency
    unsigned bestPitchPeriod =  pitchPeriodMax - bestOffset;
    float bestSeconds = secondsPerSample * (float)bestPitchPeriod;
    cout << "bestFreq " << 1.0 / bestSeconds << endl;

    // Fine tuning just focuses on three taps around the best match from the 
    // coarse search.
    int tapFineLow = std::max((int)bestOffset - 1, 0);
    int tapFineHigh = std::min((int)bestOffset + 1, (int)taps);

    for (unsigned tapOffset = tapFineLow; tapOffset <= tapFineHigh; tapOffset++) {
        energy = 0;
        corr = 0;
        for (unsigned i = 0; i < corrLen; i += 2) {
            int16_t s0 = p0[tapOffset + i];
            int16_t s1 = p1[i];
            energy += (float)s0 * (float)s0;
            corr += (float)s0 * (float)s1;
        }
        float scale = std::max(energy, minPower);
        corr /= sqrt(scale);
        if (corr >= bestCorr) {
            cout << "Matched or improved" << endl;
            bestCorr = corr;
            bestOffset = tapOffset;
        }
    }

    // Convert offset to frequency
    bestPitchPeriod =  pitchPeriodMax - bestOffset;
    bestSeconds = secondsPerSample * (float)bestPitchPeriod;
    cout << "bestFreq " << 1.0 / bestSeconds << endl;

}

int main(int,const char**) {
    test_1();
    test_2();
    return 0;
}

