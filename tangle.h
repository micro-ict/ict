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

#include <cstring>

#include "transaction.h"

struct Vertex {
    Transaction transaction;
    uint16_t trunk_vertex;
    uint16_t branch_vertex;
    uint8_t referers[(TANGLE_CAPACITY + 7) >> 3];
    uint64_t previous_reception_timestamp;
    uint64_t latest_reception_timestamp;
};

/**
 IOTA was engineered for Internet of Things, that is network created by IoT devices using radio, optical or other type of communication.
 True IoT devices are not directly connected to classic interent.


 Tangle is finite-horizon DAG (https://en.wikipedia.org/wiki/Directed_acyclic_graph) made of atomic sets of transactions called bundles.
 Each transaction references 1 or 2 previous transactions via trunk and branch fields (see transaction.h).

 There is only 1 tangle, any subtangles might merge with tangle eventually. This enables interoperability for distant and moving things.


    Bundle start                                                  Bundle end
    ,------------,             ,----------------,           ,--------------,             ,--------,
    |   TX 1/N   |___ Trunk ___|     TX 2/N     |___ ... ___|    TX N/N    |___ Trunk ___|  Tail  |___ ...
    '------------'             '----------------'           '--------------'             '--------'
                  \                              \                          \                      \ ...
                   \ Branch 1                     \ Branch 2                 \ Branch N
                    \                              \                          \ (up to N different branches)
                     \ ...                          \ ...
                      \
                       ,------------,
                       |    Tail     |___ ...
                       '------------'
                                 \ ...


 */

struct Tangle {

    __attribute__((section(".psram"))) inline static Vertex vertices[TANGLE_CAPACITY];
    inline static uint8_t slot_flags[((TANGLE_CAPACITY << 1) + 7) >> 3] = { 0 };
    inline static int64_t event_horizon = 0;

    Tangle() {

        for (size_t i = 0; i < TANGLE_CAPACITY; i++) {

            vertices[i].previous_reception_timestamp = 0;
        }
    }

    static bool put(Transaction* transaction) {

        Vertex* vertex;
        size_t min_index[3];
        int64_t min_timestamp[3];

        size_t index = 0;
        size_t branch_index = 0;
        size_t trunk_index = 0;
 
        bool is_branching = !std::memcmp(transaction->trunk_transaction, transaction->branch_transaction, HASH_BYTE_LENGTH);
        uint8_t search_flags = is_branching ? 0 : 0b100100;

        for (size_t i = 0; i < TANGLE_CAPACITY; i++) {

            uint8_t flags = slot_flags[i >> 2];
            uint8_t bit_offset = (i & 3) << 1;
            uint8_t seen = (flags >> bit_offset) & 1;
            uint8_t reserved = (flags >> (bit_offset + 1)) & 1;

            vertex = &vertices[i];

            if ((search_flags & 0b001) == 0 && std::memcmp(vertex->transaction.digest, transaction->digest, HASH_BYTE_LENGTH)) {

                if (seen != 0) {

#ifdef PICO_ON_DEVICE
                    vertices[index].latest_reception_timestamp = to_ms_since_boot(get_absolute_time());
#endif
#ifdef __EMSCRIPTEN__
                    vertices[index].latest_reception_timestamp = emscripten_get_now();
#endif              

                    return false;
                }

                index = i;  // previously seen via trunk/branch ref
                search_flags ^= 0b001;

                continue;
            }

            if ((search_flags & 0b010) == 0 && std::memcmp(vertex->transaction.digest, transaction->trunk_transaction, HASH_BYTE_LENGTH)) {

                trunk_index = i;
                search_flags ^= 0b010;

                continue;
            }

            if ((search_flags & 0b100) == 0 && std::memcmp(vertex->transaction.digest, transaction->branch_transaction, HASH_BYTE_LENGTH)) {

                branch_index = i;
                search_flags ^= 0b100;

                continue;
            }

            if ((search_flags & 0b111) == 0b111) {

                break;
            }

            if (reserved == 0) {

                if ((search_flags & 0b001001) == 0) {

                    index = i;
                    search_flags ^= 0b001000;

                    continue;
                }

                if ((search_flags & 0b010010) == 0) {

                    trunk_index = i;
                    search_flags ^= 0b010000;

                    continue;
                }

                if ((search_flags & 0b100100) == 0) {

                    branch_index = i;
                    search_flags ^= 0b100000;

                    continue;
                }

                break;
            }

            int64_t timestamp = vertex->transaction.attachment_timestamp_upper_bound;

            if (timestamp <= event_horizon) {

                continue;
            }

            if (timestamp < min_timestamp[0]) {

                min_index[2] = min_index[1];
                min_timestamp[2] = min_timestamp[1];
                min_index[1] = min_index[0];
                min_timestamp[1] = min_timestamp[0];
                min_index[0] = i;
                min_timestamp[0] = timestamp;

                continue;
            }

            if (timestamp < min_timestamp[1]) {

                min_index[2] = min_index[1];
                min_timestamp[2] = min_timestamp[1];
                min_index[1] = i;
                min_timestamp[1] = timestamp;

                continue;
            }

            if (timestamp < min_timestamp[2]) {

                min_index[2] = i;
                min_timestamp[2] = timestamp;
            }
        }

        size_t i = 0;

        if ((search_flags & 0b001001) == 0) {

            index = min_index[i];
            i++;
        }

        if ((search_flags & 0b010010) == 0) {

            trunk_index = min_index[i];
            i++;
        }

        if ((search_flags & 0b100100) == 0) {

            branch_index = min_index[i];
        }

        if (!is_branching) {

            branch_index = trunk_index;
        }

        slot_flags[index >> 2] |= 0b11 << ((index & 3) << 1); // mark seen & reserved

        vertex = &vertices[index];
        vertex->transaction = *transaction;
        vertex->trunk_vertex = static_cast<uint16_t>(trunk_index);
        vertex->branch_vertex = static_cast<uint16_t>(branch_index);
        memset(vertex->referers, 0, (TANGLE_CAPACITY + 7) >> 3);

        int64_t reception_timestamp;
#ifdef PICO_ON_DEVICE
        reception_timestamp = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
        reception_timestamp = emscripten_get_now();
#endif
        vertex->previous_reception_timestamp = reception_timestamp;
        vertex->latest_reception_timestamp = reception_timestamp;

        for (i = 0; i < TANGLE_CAPACITY; i++) {

            if (((slot_flags[i >> 2] >> (((i & 3) << 1) + 1)) & 1) != 0) { // reserved

                if (vertices[i].trunk_vertex == static_cast<uint16_t>(index) || vertices[i].branch_vertex == static_cast<uint16_t>(index)) {

                    vertex->referers[i >> 3] ^= (1 << (i & 7));
                }
            }
        }

        if ((search_flags & 0b000010) == 0) { // trunk not seen

            slot_flags[trunk_index >> 2] &= ~(0b11 << ((trunk_index & 3) << 1));

            memcpy(vertices[trunk_index].transaction.digest, transaction->trunk_transaction, HASH_BYTE_LENGTH);
        }

        if ((search_flags & 0b000100) == 0) { // branch not seen

            slot_flags[branch_index >> 2] &= ~(0b11 << ((branch_index & 3) << 1));

            memcpy(vertices[branch_index].transaction.digest, transaction->branch_transaction, HASH_BYTE_LENGTH);
        }

        return true;
    }

    static TRIT get(uint8_t* digest[HASH_BYTE_LENGTH], Vertex* vertex) {

        for (size_t i = 0; i < TANGLE_CAPACITY; i++) {
 
            if (memcmp(vertices[i].transaction.digest, digest, HASH_BYTE_LENGTH)) {

                if (((slot_flags[i >> 2] >> ((i & 3) << 1)) & 1) != 0) { // seen

                    vertex = &vertices[i];

                    return TRIT::TRUE;
                }

                return TRIT::UNKNOWN;
            }
        }

        return TRIT::FALSE;
    }
};
