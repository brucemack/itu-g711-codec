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

/**
 * Performs audio packet loss concealment using the ITU G.711 Appendix I
 * method. 
 * 
 * At the present time this implementation assumes 16 bit signed
 * PCM, 8kHz sample rate, and 10ms frame size.
 */
class Plc {
public:

    Plc();

    /**
     * Call this each time a good frame of audio is received.
     * 
     * @param inFrame The input PCM data
     * @param outFrame The output PCM data
     * @param frameLen Must be 80 at the moment. 
     */
    void goodFrame(const int16_t* inFrame, int16_t* outFrame, 
        unsigned frameLen);

    /**
     * Call this each time a frame is missed. Output will still
     * be provided using the relevant PLC algorithm.
     * 
     * @param frameLen Must be 80 at the moment. 
     */
    void badFrame(int16_t* outFrame, 
        unsigned frameLen);

    /**
     * Diagnostic, returns current pitch wavelength as estimated
     * at the start of the last erasure.
     */
    unsigned getPitchWavelength() const;

    /**
     * Returns to initial state.
     */
    void reset();

private:

    /**
     * Should be called immediately when an erasure (missed block)
     * is detected. This examines the recent history and computes
     * the pitch period that will be used for synthesis later.
     */
    void _computePitchPeriod();

    /**
     * @returns An interpolated sample from the pitch buffer, including
     * the logic for smoothing the wrap-around at the end of the 
     * buffer.
     * 
     * This has the side-effect of moving the pitch buffer pointer
     * forward so only call it once per cycle.
     */
    int16_t _getSyntheticSample();

    // The number of consecutive missing frames seen
    unsigned _erasureCount = 0;

    // Used for creating the down ramp during synthesis.
    float _attenuationRamp = 1.0;
    float _attenuationRampDelta = 0.0;

    // History is 48.75 ms
    static const unsigned _histBufLen = 390;
    int16_t _histBuf[_histBufLen];

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
    // The fixed delay in the system as a result of the lag
    // between input and output.
    const unsigned _outputLag = (pitchPeriodMax / 4);
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

