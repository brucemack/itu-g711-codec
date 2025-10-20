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
#ifndef _g711_codec_h
#define _g711_codec_h

#include <cstdint>

namespace kc1fsz {

/**
 * G.771 uLaw Encoder Function
 * 
 * @param a Is a 14-bit signed audio sample represented
 * as a 16-bit integer. The significant digits are on 
 * the low side of the 16-bit argument.
 * @returns The 8-bit character signal.
 */
uint8_t encode_ulaw(int16_t a);

/**
 * G.771 uLaw Decoder Function
 * 
 * @param c The 8-bit character signal.
 * @returns A signed 14-bit PCM value represented 
 * as a signed 16-bit integer. The 14 bits of significance
 * are on the lower end, so the range of output is 
 * -8192 -> 8192.
 */
int16_t decode_ulaw(uint8_t c);


}

#endif

