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
A command-line utility for converting G711 ulaw binary data
tp PCM text data.

./decode ../tests/clip-7-g711-ulaw.bin ../tests/clip-7a-pcm.txt 
*/
int main(int argc,const char** argv) {

    if (argc < 3) {
        cout << "Argument error" << endl;
        return -1;
    }

    ifstream infile(argv[1], ios::binary);
    ofstream outfile(argv[2]);

    unsigned count = 0;
    char b;
    while (!infile.eof()) {
        infile.read(&b, 1);
        if (infile.eof())
            break;
        outfile << decode_ulaw((uint8_t)b) << endl;
        count++;
    }

    cout << "Samples: " << count << endl;

    return 0;
}