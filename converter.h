/*
Permission is hereby granted, perpetual, worldwide, non-exclusive, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:



1. The Software cannot be used in any form or in any substantial portions for development, maintenance and for any other purposes, in the military sphere and in relation to military products, including, but not limited to:

a. any kind of armored force vehicles, missile weapons, warships, artillery weapons, air military vehicles (including military aircrafts, combat helicopters, military drones aircrafts), air defense systems, rifle armaments, small arms, firearms and side arms, melee weapons, chemical weapons, weapons of mass destruction;

b. any special software for development technical documentation for military purposes;

c. any special equipment for tests of prototypes of any subjects with military purpose of use;

d. any means of protection for conduction of acts of a military nature;

e. any software or hardware for determining strategies, reconnaissance, troop positioning, conducting military actions, conducting special operations;

f. any dual-use products with possibility to use the product in military purposes;

g. any other products, software or services connected to military activities;

h. any auxiliary means related to abovementioned spheres and products.



2. The Software cannot be used as described herein in any connection to the military activities. A person, a company, or any other entity, which wants to use the Software, shall take all reasonable actions to make sure that the purpose of use of the Software cannot be possibly connected to military purposes.



3. The Software cannot be used by a person, a company, or any other entity, activities of which are connected to military sphere in any means. If a person, a company, or any other entity, during the period of time for the usage of Software, would engage in activities, connected to military purposes, such person, company, or any other entity shall immediately stop the usage of Software and any its modifications or alterations.



4. Abovementioned restrictions should apply to all modification, alteration, merge, and to other actions, related to the Software, regardless of how the Software was changed due to the abovementioned actions.



The above copyright notice and this permission notice shall be included in all copies or substantial portions, modifications and alterations of the Software.



THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef CONVERTER_H
#define CONVERTER_H

#include <stdlib.h>
#include <string>

#define PACKET_END 0xAAAA // 2 bytes marker from unused space in trits<->bytes conversion

#define BYTE_LENGTH(N) (((N) / 9) * 2)

enum TRIT : int8_t {
    FALSE = -1,
    UNKNOWN = 0,
    TRUE = 1
};

static const std::string TRYTE_ALPHABET = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static const int8_t TRYTES_TRITS_MAPPINGS[27][3] = {
    {  0,  0,  0 }, // 9
    {  1,  0,  0 }, // A
    { -1,  1,  0 }, // B
    {  0,  1,  0 }, // C
    {  1,  1,  0 }, // D
    { -1, -1,  1 }, // E
    {  0, -1,  1 }, // F
    {  1, -1,  1 }, // G
    { -1,  0,  1 }, // H
    {  0,  0,  1 }, // I
    {  1,  0,  1 }, // J
    { -1,  1,  1 }, // K
    {  0,  1,  1 }, // L
    {  1,  1,  1 }, // M
    { -1, -1, -1 }, // N
    {  0, -1, -1 }, // O
    {  1, -1, -1 }, // P
    { -1,  0, -1 }, // Q
    {  0,  0, -1 }, // R
    {  1,  0, -1 }, // S
    { -1,  1, -1 }, // T
    {  0,  1, -1 }, // U
    {  1,  1, -1 }, // V
    { -1, -1,  0 }, // W
    {  0, -1,  0 }, // X
    {  1, -1,  0 }, // Y
    { -1,  0,  0 }  // Z
};

void bytes_to_trits(const uint8_t *bytes, const size_t byte_length, int8_t *trits) {

    for (size_t i = 0; i < byte_length >> 1; i++) {

        int16_t value = (int16_t)(bytes[0] | (bytes[1] << 8));
        bytes += 2;

        for (int j = 0; j < 9; j++) {

            int8_t remainder = value % 3;
            value /= 3;

            if (remainder == 2) {

                remainder = -1;
                value += 1;
            }

            trits[j] = remainder;
        }

        trits += 9;
    }
}

void trits_to_bytes(const int8_t *trits, const size_t trit_length, uint8_t *bytes) {

    for (size_t i = 0; i < trit_length / 9; i++) {

        int16_t value = 0;

        for (int j = 9; j-- > 0;) {

            value = value * 3 + trits[j];
        }

        // LE
        *bytes++ = value & 0xFF;
        *bytes++ = (value >> 8) & 0xFF;

        trits += 9;
    }
}

int64_t trits_to_int(const int8_t *trits, size_t length) {

    int64_t value = 0;

    for (size_t i = length; i-- > 0; ) {

        value = value * 3 + trits[i];
    }

    return value;
}


void int_to_trits(int64_t value, int8_t *trits, size_t offset, size_t length) {

    int64_t absoluteValue = llabs(value);

    while (length-- > 0) {
    
        int64_t remainder = absoluteValue % 3;
        absoluteValue /= 3;

        if (remainder > 1) {
    
            remainder = -1;
            absoluteValue++;
        }

        trits[offset++] = value < 0 ? -remainder : remainder;
    }
}

void trits_to_trytes(const int8_t *trits, const size_t trit_length, char *trytes) {

    for (size_t i = 0; i < trit_length; i += 3) {

        int8_t tryte = trits[i] + trits[i + 1] * 3 + trits[i + 2] * 9;

        if (tryte < 0) {

            tryte += 27;
        }

        *trytes++ = TRYTE_ALPHABET[tryte];
    }

    *trytes++ = 0;
}

void trytes_to_trits(const char *trytes, const size_t length, int8_t *trits) {

    for (size_t i = 0; i < length; i++) {

        size_t j = TRYTE_ALPHABET.find(trytes[i]);

        for (int k = 0; k < 3; k++) {

            trits[i * 3 + k] = TRYTES_TRITS_MAPPINGS[j][k];
        }
    }
}

#endif
