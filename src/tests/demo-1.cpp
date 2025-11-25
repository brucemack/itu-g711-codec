#include <iostream>
#include <cstdint>
#include <cmath>

#include "itu-g711-plc/Plc.h"

using namespace std;
using namespace kc1fsz;

int main(int,const char**) {

    // Instantiate the PLC. There is state maintained
    // between calls.
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

        // Put a tone into the input frame for demonstration
        for (unsigned i = 0; i < frameLen; i++) {
            inFrame[i] = 0.5 * 32767.0f * std::cos(phi);
            phi += omega;
        }

        // Here is where the PLC gets applied. Call
        // goodFrame() for a valid frame. Call badFrame()
        // when a frame is missing. In either case you'll
        // get an output frame that can sent along the 
        // audio pipeline.

        // Create a few erasures to demonstrate interpolation
        if (j == 4 || j == 5 || j >= 8)
            plc.badFrame(outFrame, frameLen);
        // Otherwise feed good audio 
        else
            plc.goodFrame(inFrame, outFrame, frameLen);

        // For display
        for (unsigned i = 0; i < frameLen; i++) 
            cout << inFrame[i] << "\t" << outFrame[i] << endl;
    }
}
