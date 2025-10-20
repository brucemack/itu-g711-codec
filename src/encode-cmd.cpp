/**
 * ITU G.711 CODEC
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
 *
 * NOT FOR COMMERCIAL USE WITHOUT PERMISSION.
 */
#include <fstream>
#include <iostream>
#include <cstdint>
#include <cassert>
#include <string>
#include "itu-g711-codec/codec.h"

using namespace std;
using namespace kc1fsz;

/*
A command-line utility for converting PCM text data to 
G711 ulaw binary data.

Convert .txt representation of a PCM recording into a 
binary G.711 representation:

./encode ../tests/clip-7-pcm.txt ../tests/clip-7-g711-ulaw.bin
*/
int main(int argc,const char** argv) {

    if (argc < 3) {
        cout << "Argument error" << endl;
        return -1;
    }

    ifstream infile(argv[1]);
    ofstream outfile(argv[2], ios::binary);

    unsigned count = 0;
    string line;
    while (getline(infile, line)) {
        count++;
        int16_t a = stoi(line);
        // The 16-bit value needs to be shifted down
        // to a 14-bit value to be compatible with 
        // the encoder.
        a = a >> 2;
        outfile << encode_ulaw(a);
    }

    cout << "Writing to: " << argv[2] << endl;
    cout << "Samples:    " << count << endl;

    return 0;
}