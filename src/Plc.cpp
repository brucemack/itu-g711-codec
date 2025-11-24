/**
 * ITU G.711 Appendix 1 PLC Algorithm
 * Copyright (C) 2025, Bruce MacKinnon 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include <cmath>
#include <algorithm>
#include <iostream>
#include <numbers>

#include "itu-g711-plc/Plc.h"

using namespace std;

namespace kc1fsz {

void Plc::test() {

    // Fill the pitch buffer with test data
    float f = 165;
    float f2 = 100;
    float omega = 2 * 3.14156 * f / sampleRate;
    float omega2 = 2 * 3.14156 * f2 / sampleRate;
    float phi = 0;
    float phi2 = 0;
    for (unsigned i = 0; i < _pitchBufLen; i++) {
        _pitchBuf[i] = 0.5 * 32767.0f * std::cos(phi) + 0.0 * 32767.0f * std::cos(phi2);
        phi += omega;
        phi2 += omega2;
    }

    _computePitchPeriod();
}

void Plc::_computePitchPeriod() {

    // Setup the anchor points for the correlation. p1 is the beginning
    // of the newest 20ms block in the pitch buffer.
    const unsigned p1 = _pitchBufLen - corrLen; 

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
        unsigned p0 = _pitchBufLen - corrLen - tapOffset; 
        for (unsigned i = 0; i < corrLen; i += step) {
            int16_t s0 = _pitchBuf[p0 + i];
            int16_t s1 = _pitchBuf[p1 + i];
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
        unsigned p0 = _pitchBufLen - corrLen - tapOffset; 
        for (unsigned i = 0; i < corrLen; i += step) {
            int16_t s0 = _pitchBuf[p0 + i];
            int16_t s1 = _pitchBuf[p1 + i];
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

    _pitchPeriod = bestOffset;
    _quarterPitchPeriod = _pitchPeriod / 4;

    // Fill the blend coefficient buffer. Here we are using 
    // a Hanning window function to minimize the spectral impact
    // of the blend.
    for (unsigned i = 0; i < _quarterPitchPeriod; i++) {
        // Set the phase so that we go through a half cycle in 
        // a quarter pitch period.
        float phi = std::numbers::pi * (float)i / 
            (float)_quarterPitchPeriod; 
        _blendCoef[i] = 0.5f - 0.5f * std::cos(phi);
    }

    // Convert offset to frequency
    //float bestSeconds = secondsPerSample * (float)bestOffset;
    //cout << "bestOffset " << bestOffset << endl;
    //cout << "bestFreq " << 1.0 / bestSeconds << endl;
    //cout << "bestCorr " << bestCorr << endl;
}

}
