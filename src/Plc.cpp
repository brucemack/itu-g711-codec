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

Plc::Plc() {
    memset(_histBuf, 0, sizeof(_histBuf));
    memset(_pitchBuf, 0, sizeof(_pitchBuf));
}

void Plc::goodFrame(const int16_t* inFrame, int16_t* outFrame, 
    unsigned frameLen) {

    // Shift history left
    memmove(_histBuf, _histBuf + _frameLen,  
        sizeof(int16_t) * (_histBufLen - _frameLen));
    // Fill in the newest block
    memcpy(_histBuf + _histBufLen - _frameLen, inFrame, 
        sizeof(int16_t) * _frameLen);

    // Is this a transition out of an erasure?
    if (_erasureCount) {
    }
    
    // Populate output with lagged data
    for (unsigned i = 0; i < _frameLen; i++)
        outFrame[i] = _histBuf[_histBufLen - _frameLen - 
            _outputLag + i];
}

void Plc::badFrame(int16_t* outFrame, 
    unsigned frameLen) {

    assert(frameLen == _frameLen);

    // In this a transition into an erasure? If so, capture the 
    // most recent history into the pitch buffer and prepare for
    // synthesis
    if (_erasureCount == 0) {
        //cout << "Switching to synthesis" << endl;
        // Move latest history into the pitch buffer
        memcpy(_pitchBuf, _histBuf + _histBufLen - _pitchBufLen,
            sizeof(int16_t) * _pitchBufLen);
        _computePitchPeriod();
    }

    // Shift history left
    memmove(_histBuf, _histBuf + _frameLen,  
        sizeof(int16_t) * (_histBufLen - _frameLen));

    _erasureCount++;

    // Populate output with interpolated data
    for (unsigned i = 0; i < _frameLen; i++)
        outFrame[i] = _getPitchBufSample();
}

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

    // Cycle through a few times 
    for (unsigned i = 0; i < _pitchBufLen; i++)     
        cout << _getPitchBufSample() << endl;
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

    // #### TODO: Look into fixed point

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
        // Any better?
        if (corr >= bestCorr) {
            bestCorr = corr;
            bestOffset = tapOffset;
        }
    }

    _pitchWavelen = bestOffset;
    _quarterPitchWavelen = _pitchWavelen / 4;

    // Start the pitch buffer pointer with the usual lag to avoid
    // a discontinuity when switching to synthesized audio. The
    // first few samples after the start of an erasure will be
    // exactly what we would have had without the erasure. This
    // will change once we enter the last 1/4 wavelength of the 
    // pitch buffer and we start to transition into the repeating
    // audio phase.
    _pitchBufPtr = _pitchBufLen - _outputLag;

    // Fill the blend coefficient buffer based on the new wavelength. 
    // Here we are using a Hanning window function to minimize the 
    // spectral impact of the blend.
    for (unsigned i = 0; i < _quarterPitchWavelen; i++) {
        float frac = (float)i / (float)_quarterPitchWavelen;
        // Set the phase so that we go through a half cycle in 
        // a quarter pitch period.
        float phi = std::numbers::pi * frac;
        _blendCoef[i] = 0.5f - 0.5f * std::cos(phi);
    }
}

int16_t Plc::_getPitchBufSample() {
    assert(_pitchBufPtr < _pitchBufLen);
    assert(_pitchWavelen * _pitchWaveCount <= _pitchBufLen);
    assert(_pitchWavelen * _pitchWaveCount <= _pitchBufPtr);
    int16_t s0 = _pitchBuf[_pitchBufPtr]; 
    int16_t s1 = _pitchBuf[_pitchBufPtr - (_pitchWavelen * _pitchWaveCount)];
    int16_t s0FadedOut = s0;
    int16_t s1FadedIn = 0;

    // Inside of the 1/4 wavelength transition period we are preparing
    // to wrap around to the start of the buffer so we want to 
    // fade out the end of the buffer and fade in the start.
    if (_pitchBufPtr >= _pitchBufLen - _quarterPitchWavelen) {
        unsigned blendPtr = _pitchBufPtr - (_pitchBufLen - _quarterPitchWavelen);
        assert(blendPtr < _quarterPitchWavelen);
        s0FadedOut = (float)s0 * (1.0 - _blendCoef[blendPtr]);
        s1FadedIn = (float)s1 * _blendCoef[blendPtr];
    }

    // Move across the pitch buffer, wrapping as needed.
    if (++_pitchBufPtr == _pitchBufLen) {
        _pitchBufPtr = _pitchBufLen - _pitchWavelen * _pitchWaveCount;
    }

    return s0FadedOut + s1FadedIn;
}

}
