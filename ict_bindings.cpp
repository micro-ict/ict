
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

#include <emscripten.h>

#include <cstdlib>
#include <iostream>
#include <string.h>

#include "transaction.h"

#define FRAME_LENGTH TRANSACTION_LENGTH

#include "ict.h"

using std::memcpy;

extern "C" {

IcT ict = IcT();

// Converter
void to_trytes(
                const int8_t *trits,
                const size_t  length,
                      char   *trytes)
{

    trits_to_trytes(trits, length, trytes);
    trytes[length / 3] = '\0';
}

void to_trits(
                const char   *trytes,
                const size_t  length,
                      int8_t *trits)
{

    trytes_to_trits(trytes, length, trits);
}

void int64_to_trits(
                     const int64_t  int64,
                     int8_t        *trits,
                     size_t         offset,
                     const size_t   length)
{

    int_to_trits(int64, trits, offset, length);
}

void trits_to_int64(
                        const int8_t  *trits,
                        const size_t   length,
                              int64_t *int64)
{

    *int64 = trits_to_int(trits, length); 
}

// Curl
void get_digest(
                    const int8_t *message,
                          size_t  message_length,
                          int8_t *digest)
{

    ict.curl.get_digest(message, 0, message_length, digest, 0);
}

// ISS
void generate_address(
                        const int8_t  *seed,
                              int64_t *index,
                        const int8_t   security,
                              int8_t  *key,
                              int8_t  *address)
{

    ict.iss.generate_address(seed, index, security, key, address);
}

void generate_signature(
                            const int8_t *bundle,
                            const int8_t *key,
                            const int8_t  security,
                                  int8_t *signature_fragments)
{

    ict.iss.generate_signature(bundle, key, security, signature_fragments);
}

bool verify_signature(
                        const int8_t *expected_address,
                        const int8_t *bundle,
                        const int8_t *signature_fragments,
                        const int8_t  security)
{

    int8_t actual_address[HASH_LENGTH];

    return ict.iss.verify_signature(expected_address, bundle, signature_fragments, security, actual_address);
}

// MSS
void generate_merkle_tree(
                            const int8_t  *seed,
                                  int64_t *index,
                            const int8_t   security)
{

    ict.mss.generate_merkle_tree(seed, index, security);
}

void generate_merkle_signature(
                                const int8_t *bundle,
                                const int8_t *seed,
                                      int8_t *signature_fragments)
{

    ict.mss.generate_merkle_signature(bundle, seed, signature_fragments);
}

void regenerate_merkle_tree(
                                const int8_t  *seed,
                                      int64_t *index)
{

    ict.mss.regenerate_merkle_tree(seed, index);
}

bool get_merkle_path(
                        int8_t* siblings)
{

    return ict.mss.get_merkle_path(siblings);
}

void get_merkle_root(
                        const size_t  leaf_index,
                        const int8_t *hash,
                        const int8_t *siblings,
                        const size_t  depth,
                              int8_t *root)
{

    ict.mss.get_merkle_root(leaf_index, hash, siblings, depth, root);
}

bool verify_merkle_signature(
                                const int8_t *root,
                                const int8_t *bundle,
                                const int8_t *signature_fragments,
                                const size_t  leaf_index,
                                      int8_t *siblings,
                                const size_t  depth,
                                const int8_t  security_level)
{

    return ict.mss.verify_merkle_signature(root, bundle, signature_fragments, leaf_index, siblings, depth, security_level);
}

void get_root_address(
                        int8_t *root)
{

    memcpy(root, ict.mss.root, HASH_LENGTH);

}

// ICT node
void transmit_frame(
                        uint8_t *data,
                        size_t   length)
{

    EM_ASM({
        if (Module.transmitFrame) {
            Module.transmitFrame($0, $1);
        }
    }, data, length);
}

void init()
{

    ict = IcT();
    ict.transmitter.packet_reception_time_ms    = 200;
    ict.transmitter.transmit_frame              = transmit_frame;
}

// Bundle construction
int8_t* bundle_transaction(
                            const TransactionDescription *transaction_description,
                            const ValidationLevel         type)
{

    return ict.ixi.bundle_transaction(transaction_description, type);
}

void finalize_bundle(
                        const int8_t  security_level,
                              int8_t *bundle)
{
    ict.ixi.finalize_bundle(security_level, bundle);
}

void set_message_or_signature(
                                const int8_t *signature,
                                const int8_t  security_level,
                                const size_t  offset)
{

    ict.ixi.set_message_or_signature(signature, security_level, offset);
}

void set_tag(
                                const int8_t *tag,
                                const size_t  offset)
{

    ict.ixi.set_tag(tag, offset);
}

void entangle(
                const int8_t *trunk_transaction,
                const int8_t *branch_transaction,
                const int8_t  security_level,
                      int8_t *transactions_copy,
                      int8_t *digests)
{

    ict.ixi.entangle(trunk_transaction, branch_transaction, security_level, transactions_copy, digests);
}

}
