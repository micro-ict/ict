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

#ifndef ISS_H
#define ISS_H

#include <stdint.h>
#include "curl.h"
#include "converter.h"
#include "adder.h"

#define SEED_LENGTH     243
#define SUBSEED_LENGTH  SEED_LENGTH

#define NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS 27
#define KEY_OR_SIGNATURE_FRAGMENT_LENGTH (HASH_LENGTH * NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS)

#define MAX_TRYTE 13
#define MIN_TRYTE -13

static const int8_t SECURITY_LEVEL_TRITS[4] = { 2, 0, 1, -1 };

struct ISS {

    Curl_729_27 *curl;

    void generate_subseed(const int8_t *seed,  const int64_t index, int8_t *subseed) {

        int8_t index_trits[SUBSEED_LENGTH];
        int_to_trits(index, index_trits, 0, SUBSEED_LENGTH);

        memcpy(subseed, seed, SUBSEED_LENGTH);
        add(subseed, index_trits, SUBSEED_LENGTH);

        curl->reset(0);
        curl->absorb(subseed, 0, SUBSEED_LENGTH);
        curl->squeeze(subseed, 0, SUBSEED_LENGTH);
    }

    void generate_key(const int8_t *subseed,  const int8_t security_level, int8_t *key) {

        // key expansion
        curl->reset(0);
        curl->absorb(subseed, 0, SUBSEED_LENGTH);
        curl->squeeze(key, 0, security_level * KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

        // reduce cryptographic assumptions
        for (size_t offset = 0; offset < (security_level * KEY_OR_SIGNATURE_FRAGMENT_LENGTH); offset += HASH_LENGTH) {

            curl->reset(0);
            curl->absorb(key, offset, HASH_LENGTH);
            curl->squeeze(key, offset, HASH_LENGTH);
        }
    }

    void generate_digests(const int8_t *key, const int8_t security_level, int8_t *digests) {

        for (size_t i = 0; i < security_level; i++) {

            int8_t buffer[KEY_OR_SIGNATURE_FRAGMENT_LENGTH];
            memcpy(buffer, key + i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH, KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

            for (int8_t j = 0; j < NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS; j++) {

                for (int8_t k = 0; k < (MAX_TRYTE - MIN_TRYTE); k++) {

                    curl->reset(0);
                    curl->absorb(buffer, j * HASH_LENGTH, HASH_LENGTH);
                    curl->squeeze(buffer, j * HASH_LENGTH, HASH_LENGTH);
                }
            }
       
            curl->reset(0);
            curl->absorb(buffer, 0, KEY_OR_SIGNATURE_FRAGMENT_LENGTH);
            curl->squeeze(digests, i * HASH_LENGTH, HASH_LENGTH);
        }
    }

    void generate_address(const int8_t *seed, int64_t *index, const int8_t security_level, int8_t *key, int8_t *address) {

        int8_t subseed_preimage[SUBSEED_LENGTH];
        int8_t subseed[SUBSEED_LENGTH];
        int8_t index_trits[SUBSEED_LENGTH];
        int8_t digests[HASH_LENGTH * 3];

        memcpy(subseed_preimage, seed, SUBSEED_LENGTH);

        // increment index
        *index = *index + 1;
        int_to_trits(*index, index_trits, 0, SUBSEED_LENGTH);

        // add subseed_preimage = seed + index
        add(subseed_preimage, index_trits, SUBSEED_LENGTH);

        while (true) {
    
            // hash(seed + index)
            curl->reset(0);
            curl->absorb(subseed_preimage, 0, SUBSEED_LENGTH);
            curl->squeeze(subseed, 0, SUBSEED_LENGTH);

            // key expansion
            generate_key(subseed, security_level, key);

            // subsequent hashing to generate digests
            generate_digests(key, security_level, digests);

            // address is hash of all digests
            curl->reset(0);
            curl->absorb(digests, 0, security_level * HASH_LENGTH);
            curl->squeeze(address, 0, HASH_LENGTH);

            if (address[0] != SECURITY_LEVEL_TRITS[security_level]) { // first trit denotes security lvl

                break;
            }

            // increment to get 1st trit matching security lvl
            *index = *index + 1;

            for (size_t i = 0; i < SUBSEED_LENGTH; i++) {

                subseed_preimage[i] = static_cast<int8_t>(subseed_preimage[i] + 1);

                if (subseed_preimage[i] > 1 ) {

                    subseed_preimage[i] = -1;
                } else {

                    break;
                }
            }
        }
    }

    void generate_signature(const int8_t *bundle, const int8_t *key, const int8_t security_level, int8_t *signature) {

        for (int8_t i = 0; i < security_level; i++) {

            // copy key fragment
            memcpy(signature + i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH, key + i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH, KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

            for (int8_t j = 0; j < NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS; j++) {

                // hash key segments to reveal
                for (int k = 0; k < (MAX_TRYTE - bundle[i * NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS + j]); k++) {

                    curl->reset(0);
                    curl->absorb(signature, i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH + j * HASH_LENGTH, HASH_LENGTH);
                    curl->squeeze(signature, i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH + j * HASH_LENGTH, HASH_LENGTH);
                }
            }
        }
    }

    bool verify_signature(const int8_t *expected_address, const int8_t *bundle, const int8_t *signature, const int8_t security_level, int8_t *actual_address) {

        int8_t digests[HASH_LENGTH * 3];
                    
        for (int8_t i = 0; i < security_level; i++) {
            int8_t buffer[KEY_OR_SIGNATURE_FRAGMENT_LENGTH];
            memcpy(buffer, signature + i * KEY_OR_SIGNATURE_FRAGMENT_LENGTH, KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

            // remaining rounds to get digests
            for (int8_t j = 0; j < NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS; j++) {

                for (int8_t k = bundle[i * NUMBER_OF_KEY_OR_SIGNATURE_SEGMENTS + j] - MIN_TRYTE; k-- > 0;) {

                    curl->reset(0);
                    curl->absorb(buffer, j * HASH_LENGTH, HASH_LENGTH);
                    curl->squeeze(buffer, j * HASH_LENGTH, HASH_LENGTH);
                }
            }

            curl->reset(0);
            curl->absorb(buffer, 0, KEY_OR_SIGNATURE_FRAGMENT_LENGTH);
            curl->squeeze(digests, i * HASH_LENGTH, HASH_LENGTH);
        }

        curl->reset(0);
        curl->absorb(digests, 0, security_level * HASH_LENGTH);
        curl->squeeze(actual_address, 0, HASH_LENGTH);

        return (memcmp(actual_address, expected_address, HASH_LENGTH) == 0);
    }
};

#endif
