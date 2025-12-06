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

#ifndef MSS_H
#define MSS_H

#include <stdint.h>
#include "curl.h"
#include "iss.h"

template <size_t D>
struct MSS {

public:
    ISS *iss;
    int8_t root[HASH_LENGTH];

private:
    int8_t security;
    int64_t key_indices[1 << D];
    int8_t nodes[((1 << (D + 1)) - 1) * HASH_LENGTH];
    size_t current_index = -1;

    void compute_nodes() {

        size_t offset = 0;
        size_t level_size = 1 << D;

        while (level_size > 1) {

            size_t next_offset = offset + level_size * HASH_LENGTH;

            for (size_t i = 0; i < level_size; i += 2) {

                iss->curl->reset(0);
                iss->curl->absorb(nodes, offset + i * HASH_LENGTH, HASH_LENGTH);
                iss->curl->absorb(nodes, offset + (i + 1) * HASH_LENGTH, HASH_LENGTH);
                iss->curl->squeeze(nodes, next_offset + (i/2) * HASH_LENGTH, HASH_LENGTH);
            }

            offset = next_offset;
            level_size >>= 1;
        }

        memcpy(root, nodes + offset, HASH_LENGTH);
        current_index = -1;
    }

public:
    void generate_merkle_tree(const int8_t *seed, int64_t *index, const int8_t security_level) {

        security = security_level;

        int8_t key[3 * KEY_OR_SIGNATURE_FRAGMENT_LENGTH];

        for (size_t i = 0; i < (1 << D); ++i) {

            iss->generate_address(seed, index, security, key, nodes + (i * HASH_LENGTH));
            key_indices[i] = *index;
        }

        memset(key, 0, 3 * KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

        compute_nodes();
    }

    void generate_merkle_signature(const int8_t *bundle, const int8_t *seed, int8_t *signature_fragments) {

        int8_t subseed[SUBSEED_LENGTH];
        int8_t key[KEY_OR_SIGNATURE_FRAGMENT_LENGTH * 3];
    
        iss->generate_subseed(seed, key_indices[++current_index], subseed);
        iss->generate_key(subseed, security, key);
        iss->generate_signature(bundle, key, security, signature_fragments);
    }

    void regenerate_merkle_tree(const int8_t *seed, int64_t *index) {

        if (current_index >= 0) {

            int8_t key[3 * KEY_OR_SIGNATURE_FRAGMENT_LENGTH];
            iss->generate_address(seed, index, security, key, nodes + current_index * HASH_LENGTH);
            memset(key, 0, 3 * KEY_OR_SIGNATURE_FRAGMENT_LENGTH);

            key_indices[current_index] = *index;

            if (current_index == ((1 << D) - 1)) {

                compute_nodes();
            }
        }
    }

    bool get_merkle_path(int8_t* siblings) {

        if (current_index < 0 || current_index >= (1 << D)) {
            return false;
        }

        size_t offset = 0;
        size_t index = current_index;
        size_t level_size = 1 << D;

        for (size_t level = 0; level < D; ++level) {

            size_t sibling_index = index ^ 1;

            memcpy(siblings + level * HASH_LENGTH, nodes + (offset + sibling_index * HASH_LENGTH), HASH_LENGTH);

            offset += level_size * HASH_LENGTH;
            index >>= 1;
            level_size >>= 1;
        }

        return true;
    }

    void get_merkle_root(const size_t leaf_index, const int8_t *hash, const int8_t *siblings, const size_t depth, int8_t *root) {

        memcpy(root, hash, HASH_LENGTH);

        size_t leaf_index_copy = leaf_index;

        for (size_t offset = 0; offset < depth * HASH_LENGTH; offset += HASH_LENGTH) {

            iss->curl->reset(0);

            if ((leaf_index_copy & 1) == 0) {

                iss->curl->absorb(root, 0, HASH_LENGTH);
                iss->curl->absorb(siblings, offset, HASH_LENGTH);
            } else {

                iss->curl->absorb(siblings, offset, HASH_LENGTH);
                iss->curl->absorb(root, 0, HASH_LENGTH);
            }
            iss->curl->squeeze(root, 0, HASH_LENGTH);

            leaf_index_copy = leaf_index_copy >> 1;
        }
    }

    bool verify_merkle_signature(const int8_t *root, const int8_t *bundle, const int8_t *signature_fragments, const size_t leaf_index, const int8_t *siblings, const size_t depth, const int8_t security_level) {

        int8_t hash[HASH_LENGTH];

        int8_t expected_address[HASH_LENGTH];
        expected_address[0] = 2;

        iss->verify_signature(expected_address, bundle, signature_fragments, security_level, hash);

        get_merkle_root(leaf_index, hash, siblings, depth, expected_address);

        return (memcmp(expected_address, root, HASH_LENGTH) == 0);
    }
};

#endif
