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

#include "random.h"

template <size_t L>
struct Transmitter {

    Tangle* tangle;

    uint64_t number_of_transmitted_bytes = 0;
    uint64_t number_of_transmitted_transactions = 0;
    uint32_t packet_reception_time_ms = 0;

    void (*transmit_frame)(uint8_t*, size_t);

    void broadcast_transaction(const uint8_t *bytes) {

        size_t number_of_skipped_bytes = 0;
        for (; number_of_skipped_bytes < TRANSACTION_BYTE_LENGTH; number_of_skipped_bytes += HASH_BYTE_LENGTH) {

            size_t i = 0;
            for (; i < HASH_BYTE_LENGTH; i++) {

                if (bytes[number_of_skipped_bytes + i] != 0) {

                    break;
                }
            }

            if (i < HASH_BYTE_LENGTH) {

                break;
            }
        }

        size_t number_of_transmitted_bytes = TRANSACTION_BYTE_LENGTH - number_of_skipped_bytes;
        size_t number_of_frames = (number_of_transmitted_bytes + L - 1) / L;

        for (size_t i = number_of_frames; i > 0; --i) {

            uint8_t frame[L];
            memset(frame, 0, L);
            memcpy(frame, bytes + (i - 1) * L, L);

            transmit_frame(frame, L);

            number_of_transmitted_bytes += L;
        }

#ifdef PICO_ON_DEVICE
        uint8_t frame[L] = { 0 };
        frame[0] = (PACKET_END >> 8) & 0xFF;
        frame[1] = PACKET_END & 0xFF;

        transmit_frame(frame, 2);
        number_of_transmitted_bytes += 2;
#endif
        number_of_transmitted_transactions++;
    }

    void transmit(bool ignore_timestamps) {

        for (size_t i = 0; i < TANGLE_CAPACITY; i++) {

            if (tangle->vertices[i].previous_reception_timestamp != 0) {

                if (ignore_timestamps || (tangle->vertices[i].latest_reception_timestamp == tangle->vertices[i].previous_reception_timestamp)) {

                uint64_t now;
#ifdef PICO_ON_DEVICE
                now = to_ms_since_boot(get_absolute_time());
#elif defined(__EMSCRIPTEN__)
                now = emscripten_get_now();
#endif   

                    if (ignore_timestamps || ((now - tangle->vertices[i].previous_reception_timestamp) >= random_from_range(DISSEMINATION_ARGUMENT_ALPHA * packet_reception_time_ms, DISSEMINATION_ARGUMENT_BETA * packet_reception_time_ms))) {

                        tangle->vertices[i].latest_reception_timestamp = now;
 
                        uint8_t bytes[TRANSACTION_BYTE_LENGTH];
                        tangle->vertices[i].transaction.dump(bytes);

                        broadcast_transaction(bytes);

                        if (ignore_timestamps) {

                            return;
                        }
                    }
                }
            }
        }
    }
};
