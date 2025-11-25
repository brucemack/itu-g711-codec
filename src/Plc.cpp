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
#include <cassert>
#include <cstring>

#include "itu-g711-plc/Plc.h"

using namespace std;

namespace kc1fsz {

Plc::Plc() {
    reset();
}

void Plc::goodFrame(const int16_t* inFrame, int16_t* outFrame, 
    unsigned frameLen) {

    // Shift history left
    memmove(_histBuf, _histBuf + _frameLen,  
        sizeof(int16_t) * (_histBufLen - _frameLen));
    // Fill in the newest frame (far right)
    memcpy(_histBuf + _histBufLen - _frameLen, inFrame, 
        sizeof(int16_t) * _frameLen);

    // Is this a transition out of an erasure?
    if (_erasureCount > 0) {
        // For the lag period, keep flowing the synthetic data 
        // (need to catch up to the start of the new frame).
        unsigned i = 0;
        for (; i < _outputLag; i++) {
            int16_t s = _getSyntheticSample();
            outFrame[i] = s;
            // We also plug the synthetic value into the history 
            // buffer in the place that it would have come from
            // if everything was going well. This may be used 
            // if we quickly switch back into an erasure
            _histBuf[_histBufLen - _frameLen - _outputLag + i] = s;
        }

        // After the lag period we fade from the synthetic data
        // over to the real data. The length of this period is 1/4
        // wavelength for the first 10m erasure and 4ms (32 samples)
        // for each additional erasure, not to exceed the length 
        // of the frame.
        unsigned fadeLen = _quarterPitchWavelen + 32 * (_erasureCount - 1);
        // Make sure the fade doesn't extend past this frame. And
        // remember that we've already used _outputLag from the frame.
        fadeLen = std::min(fadeLen, _frameLen - _outputLag);

        // Build the blend coefficients. Using a Hamming window here,
        // but a triangle could be used if there are efficiency 
        // concerns.
        const unsigned blendCoefLen = _frameLen - _outputLag; 
        assert(blendCoefLen == 80 - 30);
        float blendCoef[50];
        for (unsigned j = 0; j < fadeLen; j++) {
            float frac = (float)j / (float)fadeLen;
            // Set the phase so that we go through a half cycle 
            float phi = std::numbers::pi * frac;
            assert(j < blendCoefLen);
            blendCoef[j] = 0.5f - 0.5f * std::cos(phi);
        }
        
        // Build the blend during the fade period
        for (unsigned f = 0; f < fadeLen; f++, i++) {
            float s0FadedOut = 
                (float)_getSyntheticSample() * (1.0 - blendCoef[f]);
            float s1FadedIn = (float)_histBuf[_histBufLen - _frameLen - 
                _outputLag + i] * blendCoef[f];
            int16_t s = s0FadedOut + s1FadedIn;
            outFrame[i] = s;
            // We also plug the synthetic value into the history 
            // buffer in the place that it would have come from
            // if everything was going well. This may be used 
            // if we quickly switch back into an erasure
            _histBuf[_histBufLen - _frameLen - _outputLag + i] = s;
        }

        // And anything left is just handled the normal way.
        for (; i < _frameLen; i++)
            outFrame[i] = _histBuf[_histBufLen - _frameLen - 
                _outputLag + i];
        _erasureCount = 0;
    }
    else {
        // Populate output with lagged input data
        for (unsigned i = 0; i < _frameLen; i++)
            outFrame[i] = _histBuf[_histBufLen - _frameLen - 
                _outputLag + i];
    }
}

void Plc::badFrame(int16_t* outFrame, 
    unsigned frameLen) {

    assert(frameLen == _frameLen);
    
    _erasureCount++;

    // In this a transition into an erasure? If so, capture the 
    // most recent history into the pitch buffer and prepare for
    // synthesis
    if (_erasureCount == 1) {
        // Move latest history into the pitch buffer
        memcpy(_pitchBuf, _histBuf + _histBufLen - _pitchBufLen,
            sizeof(int16_t) * _pitchBufLen);
        _computePitchPeriod();
        _attenuationRamp = 1.0;
        _attenuationRampDelta = 0;
    } 
    else if (_erasureCount == 2) {
        // We change the number of wavelengths but the 
        // pointer (phase) is unchanged to avoid any
        // discontinuity.
        _pitchWaveCount = 2;
        // Once we hit the second erasure we turn on the attenuation.
        // The specification requires 20% per 10ms, so that means
        // 0.2 for every frame or 0.2 / 80 = 0.0025 for every sample.
        _attenuationRampDelta = -0.2 / (float)_frameLen;
    }
    else if (_erasureCount == 3) {
        // We change the number of wavelengths but the 
        // pointer (phase) is unchanged to avoid any
        // discontinuity.
        _pitchWaveCount = 3;
    }

    // NOTE: There is no further update the wavelength count
    // after the third erasure.

    // Shift history left
    memmove(_histBuf, _histBuf + _frameLen,  
        sizeof(int16_t) * (_histBufLen - _frameLen));

    // Populate output with interpolated data
    for (unsigned i = 0; i < _frameLen; i++) {
        int16_t s = _getSyntheticSample();
        outFrame[i] = s;
        // We also plug the synthetic value into the history 
        // buffer in the place that it would have come from
        // if everything was going well. This may be used 
        // if we quickly switch back into an erasure.
        _histBuf[_histBufLen - _frameLen - _outputLag + i] = s;
    }
}

unsigned Plc::getPitchWavelength() const {
    return _pitchWavelen;
}

void Plc::reset() {
    memset(_histBuf, 0, sizeof(_histBuf));
    memset(_pitchBuf, 0, sizeof(_pitchBuf));
    memset(_blendCoef, 0, sizeof(_blendCoef));
    _pitchWavelen = 0;
    _erasureCount = 0;
    _attenuationRamp = 1.0;
    _attenuationRampDelta = 0.0;
    _pitchBufPtr = 0;
    _quarterPitchWavelen = 0;
    _pitchWaveCount = 1;
}

void Plc::_computePitchPeriod() {

    // Setup the anchor points for the correlation. p1 is the beginning
    // of the newest 20ms block in the pitch buffer.
    const unsigned p1 = _pitchBufLen - corrLen; 

    unsigned tapOffsetLow = pitchPeriodMin;
    unsigned tapOffsetHigh = pitchPeriodMax;
    float bestCorr = 0;
    unsigned bestOffset = tapOffsetHigh;
    unsigned step = 2;

    // #### TODO: Look into fixed point

    // During the coarse search we test every other tap. The scan starts
    // from the longest pitch period and ends at the highest pitch period.
    for (unsigned tapOffset = tapOffsetHigh; tapOffset >= tapOffsetLow; 
        tapOffset -= step) {
        float energy = 0;
        float corr = 0;
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
        if (corr > bestCorr) {
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
    bestOffset = tapOffsetHigh;
    bestCorr = 0;

    for (unsigned tapOffset = tapOffsetHigh; tapOffset >= tapOffsetLow; 
        tapOffset -= step) {
        float energy = 0;
        float corr = 0;
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

    // Convert 
    //cout << "Freq " << 8000.0 / (float)_pitchWavelen << endl;

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

int16_t Plc::_getSyntheticSample() {

    assert(_pitchBufPtr < _pitchBufLen);
    assert(_pitchWavelen * _pitchWaveCount <= _pitchBufLen);
    int16_t s0 = _pitchBuf[_pitchBufPtr]; 
    int16_t s0FadedOut = s0;
    int16_t s1FadedIn = 0;

    // Inside of the 1/4 wavelength transition period we are preparing
    // to wrap around to the start of the buffer so we want to 
    // fade out the end of the buffer and fade in the start.
    if (_pitchBufPtr >= _pitchBufLen - _quarterPitchWavelen) {
        assert(_pitchWavelen * _pitchWaveCount <= _pitchBufPtr);
        int16_t s1 = _pitchBuf[_pitchBufPtr - (_pitchWavelen * _pitchWaveCount)];
        unsigned blendPtr = _pitchBufPtr - (_pitchBufLen - _quarterPitchWavelen);
        assert(blendPtr < _quarterPitchWavelen);
        s0FadedOut = (float)s0 * (1.0 - _blendCoef[blendPtr]);
        s1FadedIn = (float)s1 * _blendCoef[blendPtr];
    }

    // Move across the pitch buffer, wrapping as needed.
    if (++_pitchBufPtr == _pitchBufLen) {
        assert(_pitchWavelen * _pitchWaveCount < _pitchBufLen);
        _pitchBufPtr = _pitchBufLen - _pitchWavelen * _pitchWaveCount;
    }

    // Apply the attenuation
    float result = (s0FadedOut + s1FadedIn) * _attenuationRamp;
    _attenuationRamp += _attenuationRampDelta;
    if (_attenuationRamp < 0)
        _attenuationRamp = 0;
    if (_attenuationRamp > 1.0)
        _attenuationRamp = 1.0;

    return result;
}

}
