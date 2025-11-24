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

    void test();

private:

    void _computePitchPeriod();

    unsigned _pitchBufPtr = 0;
    unsigned _pitchPeriod = 0;
    unsigned _quarterPitchPeriod = 0;

    const float sampleRate = 8000;
    const float secondsPerSample = 1.0 / sampleRate;
    // A 10ms block at 8kHz
    const unsigned blockSize = 80;
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
    int16_t _blendCoef[pitchPeriodMax / 4];
};

}

