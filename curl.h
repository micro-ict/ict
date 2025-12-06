/*
(c) Come-from-Beyond

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

#ifndef CURL_729_27_H
#define CURL_729_27_H

#define HASH_LENGTH 243

struct Curl_729_27 {

    static constexpr size_t STATE_LENGTH = HASH_LENGTH * 3;
    static constexpr size_t NUMBER_OF_ROUNDS = 27;

    uint8_t state[STATE_LENGTH];
    uint8_t scratchpad[STATE_LENGTH];

    void reset(ssize_t message_length) {

        int8_t message_length_trits[HASH_LENGTH];
        memset(message_length_trits, 0, HASH_LENGTH);

        if (message_length > 0) {

            int64_t absolute_value = static_cast<int64_t>(message_length);
            size_t i = 0;

            do {
 
                int64_t remainder = absolute_value % 3;
                absolute_value = absolute_value / 3;
 
                if (remainder > 1) {
 
                    remainder = -1;
                    absolute_value += 1;
                }
                message_length_trits[i++] = static_cast<int8_t>(remainder);
    
            } while (i < HASH_LENGTH);
        }

        for (size_t i = 0; i < HASH_LENGTH; i++) {

            state[0 * HASH_LENGTH + i] = static_cast<uint8_t>((i + 1) % 3);
            state[1 * HASH_LENGTH + i] = static_cast<uint8_t>((message_length_trits[i] + 1) % 3);
            state[2 * HASH_LENGTH + i] = static_cast<uint8_t>((i + 1) % 3);
        }
    }

    void absorb(const int8_t* trits, size_t offset, ssize_t length) {

        do {

            for (size_t i = 0; i < (length < HASH_LENGTH ? length : HASH_LENGTH); i++) {

                state[i] = static_cast<uint8_t>(trits[offset++] + 1);
            }
            transform();

        } while ((length -= HASH_LENGTH) > 0);
    }

    void squeeze(int8_t* trits, size_t offset, ssize_t length) {

        do {

            for (size_t i = 0; i < (length < HASH_LENGTH ? length : HASH_LENGTH); i++) {

                trits[offset++] = static_cast<int8_t>(state[i]) - 1;
            }
            transform();

        } while ((length -= HASH_LENGTH) > 0);
    }

    static void get_digest(const int8_t* message_trits, size_t message_offset, ssize_t message_length, int8_t* digest_trits, size_t digest_offset) {
 
        Curl_729_27 curl = Curl_729_27();
        curl.reset(message_length);
        curl.absorb(message_trits, message_offset, message_length);

        for (size_t i = 0; i < HASH_LENGTH; i++) {

            digest_trits[digest_offset++] = static_cast<int8_t>(curl.state[i]) - 1;
        }
    }

private:

    static constexpr uint8_t LUT_0[43] = {
        1, 0, 0, 0, 1, 2, 2, 0, 2, 0, 1, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 2, 1,
        0, 0, 1, 2, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 1, 0, 0, 2, 0
    };

    static constexpr uint8_t LUT_1[43] = {
        1, 0, 2, 0, 0, 1, 1, 0, 0, 2, 2, 0, 0, 0, 0, 0, 1, 1, 0, 0, 2, 2, 0,
        0, 2, 1, 1, 0, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1, 2, 0, 1, 2, 0
    };

    static constexpr uint8_t LUT_2[43] = {
        1, 1, 2, 0, 0, 1, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 1, 2, 1,
        0, 1, 0, 2, 0, 0, 0, 0, 0, 0, 2, 1, 0, 2, 1, 2, 0, 2, 1, 0
    };

    void transform() {

        for (size_t round = 0; round < NUMBER_OF_ROUNDS; ++round) {

            // Step 1
            for (size_t a = 0; a < HASH_LENGTH; ++a) {

                size_t b = a + 243;
                if (b >= STATE_LENGTH) b -= STATE_LENGTH;
                size_t c = a + 486;
                if (c >= STATE_LENGTH) c -= STATE_LENGTH;

                size_t index = static_cast<size_t>(state[a] | (state[b] << 2) | (state[c] << 4));

                scratchpad[a] = LUT_0[index];
                scratchpad[b] = LUT_1[index];
                scratchpad[c] = LUT_2[index];
            }

            // Step 2
            size_t j = 81;
            size_t a = 0;

            while (a < STATE_LENGTH) {

                size_t b = a + 81;
                if (b >= STATE_LENGTH) b -= STATE_LENGTH;
                size_t c = a + 162;
                if (c >= STATE_LENGTH) c -= STATE_LENGTH;

                size_t index = static_cast<size_t>(scratchpad[a] | (scratchpad[b] << 2) | (scratchpad[c] << 4));

                state[a] = LUT_0[index];
                state[b] = LUT_1[index];
                state[c] = LUT_2[index];

                a++;
                j--;

                if (j == 0) {

                    j = 81;
                    a = c + 1;
                }
            }

            // Step 3
            j = 27;
            a = 0;

            while (a < STATE_LENGTH) {

                size_t b = a + 27;
                if (b >= STATE_LENGTH) b -= STATE_LENGTH;
                size_t c = a + 54;
                if (c >= STATE_LENGTH) c -= STATE_LENGTH;

                size_t index = static_cast<size_t>(state[a] | (state[b] << 2) | (state[c] << 4));

                scratchpad[a] = LUT_0[index];
                scratchpad[b] = LUT_1[index];
                scratchpad[c] = LUT_2[index];

                a++;
                j--;

                if (j == 0) {

                    j = 27;
                    a = c + 1;
                }
            }

            // Step 4
            j = 9;
            a = 0;

            while (a < STATE_LENGTH) {

                size_t b = a + 9;
                if (b >= STATE_LENGTH) b -= STATE_LENGTH;
                size_t c = a + 18;
                if (c >= STATE_LENGTH) c -= STATE_LENGTH;

                size_t index = static_cast<size_t>(scratchpad[a] | (scratchpad[b] << 2) | (scratchpad[c] << 4));

                state[a] = LUT_0[index];
                state[b] = LUT_1[index];
                state[c] = LUT_2[index];

                a++;
                j--;

                if (j == 0) {

                    j = 9;
                    a = c + 1;
                }
            }

            // Step 5
            j = 3;
            a = 0;

            while (a < STATE_LENGTH) {

                size_t b = a + 3;
                if (b >= STATE_LENGTH) b -= STATE_LENGTH;
                size_t c = a + 6;
                if (c >= STATE_LENGTH) c -= STATE_LENGTH;

                size_t index = static_cast<size_t>(state[a] | (state[b] << 2) | (state[c] << 4));

                scratchpad[a] = LUT_0[index];
                scratchpad[b] = LUT_1[index];
                scratchpad[c] = LUT_2[index];

                a++;
                j--;

                if (j == 0) {

                    j = 3;
                    a = c + 1;
                }
            }

            // Step 6
            a = 0;

            while (a < STATE_LENGTH) {

                size_t index = static_cast<size_t>(scratchpad[a] | (scratchpad[a + 1] << 2) | (scratchpad[a + 2] << 4));

                state[a] = LUT_0[index];
                state[a + 1] = LUT_1[index];
                state[a + 2] = LUT_2[index];

                a += 3;
            }
        }
    }
};

#endif
