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
#pragma once

#include <cstdint>

namespace kc1fsz {

class Plc {
public:

    Plc();

    /**
     * Call this each time a good frame of audio is received.
     * @param inFrame The input PCM data
     * @param outFrame The output PCM data
     * @param frameLen 
     */
    void goodFrame(const int16_t* inFrame, int16_t* outFrame, 
        unsigned frameLen);

    /**
     * Call this each time a frame is missed. Output will still
     * be provided using the relevant PLC algorithm.
     */
    void badFrame(int16_t* outFrame, 
        unsigned frameLen);

    void test();

private:

    void _computePitchPeriod();

    /**
     * @returns An interpolated sample from the pitch buffer, taking 
     * into account all of the rules related to erasure count, etc.
     * This has the side-effect of moving the pitch buffer pointer
     * forward so only call it once per cycle.
     * 
     * This function includes the logic to blend the wrap-around
     * discontinuity.
     */
    int16_t _getPitchBufSample();

    unsigned _erasureCount = 0;

    // History is 48.75 ms
    static const unsigned _histBufLen = 390;
    int16_t _histBuf[_histBufLen];
    const unsigned _outputLag = (pitchPeriodMax / 4);

    unsigned _pitchBufPtr = 0;
    // These are set by the pitch determination function
    unsigned _pitchWavelen = 0;
    unsigned _quarterPitchWavelen = 0;
    // The number of wavelengths in the synthesis. This depends 
    // on how many erasures have happened so far.
    unsigned _pitchWaveCount = 1;

    const float sampleRate = 8000;
    const float secondsPerSample = 1.0 / sampleRate;
    // A 10ms block at 8kHz
    const unsigned _frameLen = 80;
    // The period of a 66 Hz pitch
    static const unsigned pitchPeriodMax = 120; 
    // The period of a 200 Hz pitch
    const unsigned pitchPeriodMin = 40; 
    // The length of the correlation period used when searching for the pitch
    const unsigned corrLen = 160;
    const float minPower = 250;
    // The pitch buffer is long enough for three complete cycles
    // at the lowest pitch frequency.
    // ### TODO: CHECK THIS
    static const unsigned _pitchBufLen = 3 * pitchPeriodMax;
    int16_t _pitchBuf[_pitchBufLen];
    // Holds the blend curve that is used to transition between 
    // discontinuous signals. This buffer goes from 0.0->1.0 so
    // you may need subtract it from 1.0 to produce the ramp-down.
    float _blendCoef[pitchPeriodMax / 4];
};

}

