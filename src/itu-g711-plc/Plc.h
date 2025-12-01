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
 * PCM, and 10ms frame size.  Maximum sample rate is 48K.
 */
class Plc {
public:

    Plc();

    /**
     * Changes the sample rate and causes a reset.
     * @param Audio sample rate in Hertz
     */
    void setSampleRate(unsigned hz);

    /**
     * Call this each time a good 10m frame of audio is received.
     * Each call will consume frameLen samples and will produce
     * another frameLen samples.
     * 
     * @param inFrame The input PCM data
     * @param outFrame The output PCM data
     * @param frameLen Depends on the sample rate, but must be 10ms of data
     */
    void goodFrame(const int16_t* inFrame, int16_t* outFrame, 
        unsigned frameLen);

    /**
     * Call this each time a 10ms frame is missed. Output will still
     * be provided using the relevant PLC algorithm.
     * 
     * @param frameLen Depends on the sample rate, but must be 10ms of data
     */
    void badFrame(int16_t* outFrame, unsigned frameLen);

    /**
     * Diagnostic, returns current pitch wavelength as estimated
     * at the start of the last erasure.
     */
    unsigned getPitchWavelength() const;

    /**
     * Returns to the initial state.
     */
    void reset();

private:

    // These constants are used to pre-allocate the largest possible work 
    // ares. They have been scaled assuming a maximum sample rate of 48K.
    //
    // IMPORTANT: Must be evenly divisible by 4.
    static const MAX_PITCH_PERIOD_LEN = 120 * 6;
    static const MAX_HIST_BUF_LEN = MAX_PITCH_PERIOD_LEN * 3.25;

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

    const float sampleRate = 8000;
    // The number of consecutive missing frames seen
    unsigned _erasureCount = 0;
    // Used for creating the down ramp during synthesis. This
    // is the current attenuation level:
    float _attenuationRamp = 1.0;
    // This is the amount the attenuation should be adjusted
    // on each sample:
    float _attenuationRampDelta = 0.0;

    // History is 48.75 ms. This is 3.25 maximum pitch periods.
    // (The lowest frequency is 66 Hz, which is 120 samples at 8kHz. 
    // 120 * 3.25=390)
    static const unsigned _histBufLen = 390;
    int16_t _histBuf[MAX_HIST_BUF_LEN];

    // This is used during synthesis. Points to the current
    // synthesized sample. This moves across the pitch 
    // buffer in circular fashion.
    unsigned _pitchBufPtr = 0;
    // These are set by the pitch determination function.
    // The dominant pitch wavelength in samples
    unsigned _pitchWavelen = 0;
    // 1/4 of above (used a lot)
    unsigned _quarterPitchWavelen = 0;
    // The number of wavelengths in the synthesis. This depends 
    // on how many erasures have happened so far.
    unsigned _pitchWaveCount = 1;
    // The number of samples in each 10ms block at 8kHz
    const unsigned _frameLen = 80;
    // The period of a 66 Hz pitch - the lowest fundamental
    // we will track.
    static const unsigned pitchPeriodMax = 120; 
    // The period of a 200 Hz pitch - the highest fundamental
    // we will track.
    const unsigned pitchPeriodMin = 40; 
    // The fixed delay in the system as a result of the lag
    // between input and output.
    const unsigned _outputLag = (pitchPeriodMax / 4);
    // The length of the correlation period used when searching for the pitch
    const unsigned corrLen = 160;
    const float minPower = 250;
    // The pitch buffer is long enough for three complete cycles
    // at the lowest pitch frequency.
    static const unsigned _pitchBufLen = 390;
    int16_t _pitchBuf[MAX_HIST_BUF_LEN];
    // Holds the blend curve that is used to transition between 
    // discontinuous signals. This buffer goes from 0.0->1.0 so
    // you will need subtract it from 1.0 to produce the ramp-down.
    float _blendCoef[MAX_PITCH_PERIOD_LEN / 4];
};

}

