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

typedef enum {
    ATTACHMENT_VALIDATION =  0,
    BUNDLE_VALIDATION     =  1,
    REGISTRY_VALIDATION   = -1
} ValidationLevel;

struct TransactionDescription {

    int8_t extra_data_digest[HASH_LENGTH];
    int8_t address[HASH_LENGTH];
    int64_t energy;
    int64_t issuance_timestamp;
    int64_t timelock_lower_bound;
    int64_t timelock_upper_bound;
};

struct IxI {

    Curl_729_27 *curl;
    Tangle *tangle;
    Transmitter<FRAME_LENGTH> *transmitter;

    ValidationLevel transaction_types[MAX_OWN_BUNDLE_SIZE];
    int8_t transactions[MAX_OWN_BUNDLE_SIZE][TRANSACTION_LENGTH];
    bool _final = 0;
    size_t length = 0;

    int8_t* bundle_transaction(const TransactionDescription *description, const ValidationLevel type) {

        memset(transactions[length], 0, TRANSACTION_LENGTH);
        memcpy(transactions[length] + EXTRA_DATA_DIGEST_OFFSET, description->extra_data_digest, HASH_LENGTH);
        memcpy(transactions[length] + ADDRESS_OFFSET, description->address, HASH_LENGTH);

        int_to_trits(description->energy, transactions[length], ENERGY_OFFSET, ENERGY_LENGTH);
        int_to_trits(description->issuance_timestamp, transactions[length], ISSUANCE_TIMESTAMP_OFFSET, ISSUANCE_TIMESTAMP_LENGTH);
        int_to_trits(description->timelock_lower_bound, transactions[length], TIMELOCK_LOWER_BOUND_OFFSET, TIMELOCK_LOWER_BOUND_LENGTH);
        int_to_trits(description->timelock_upper_bound, transactions[length], TIMELOCK_UPPER_BOUND_OFFSET, TIMELOCK_UPPER_BOUND_LENGTH);

        transaction_types[length] = type;

        return transactions[length++];
    }

    bool verify_bundle(const int8_t *bundle, const int8_t security_level) {

        if (security_level == 0) {

            return true;
        }

        for (size_t i = 0; i < security_level; i++) {

            int64_t sum = 0;

            for (size_t j = 0; j < (HASH_LENGTH / 3); j++) {

                sum += bundle[i * (HASH_LENGTH / 3) + j];
            }

            if (sum != 0) {

                return false;
            }
        }

        return true;
    }

    void finalize_bundle(const int8_t security_level, int8_t *bundle) {

        if (_final) {

            return;
        }

        _final = true;

        uint64_t start;
#ifdef PICO_ON_DEVICE
        start = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
        start = emscripten_get_now();
#endif
        uint64_t number_of_tries = 1;

        int8_t essence[MAX_OWN_BUNDLE_SIZE * TRANSACTION_ESSENCE_LENGTH];
        memset(essence, 0, length * TRANSACTION_ESSENCE_LENGTH);

        for (size_t i = 0; i < length; i++) {
            memcpy(essence + i * TRANSACTION_ESSENCE_LENGTH, transactions[i] + TRANSACTION_ESSENCE_OFFSET, TRANSACTION_ESSENCE_LENGTH);
        }

        random(BUNDLE_NONCE_LENGTH, essence + (BUNDLE_NONCE_OFFSET - TRANSACTION_ESSENCE_OFFSET));

        curl->get_digest(essence, 0, length * TRANSACTION_ESSENCE_LENGTH, bundle, 0);


        while (verify_bundle(bundle, security_level) == false) {

            for (size_t i = 0; i < BUNDLE_NONCE_LENGTH; i++) {

                essence[BUNDLE_NONCE_OFFSET - TRANSACTION_ESSENCE_OFFSET + i]++;

                if (essence[BUNDLE_NONCE_OFFSET - TRANSACTION_ESSENCE_OFFSET + i] > 1) {

                    essence[BUNDLE_NONCE_OFFSET - TRANSACTION_ESSENCE_OFFSET + i] = -1;
                } else {

                    break;
                }
            }

            curl->get_digest(essence, 0, length * TRANSACTION_ESSENCE_LENGTH, bundle, 0);

            number_of_tries++;
        }
        memcpy(transactions[0] + BUNDLE_NONCE_OFFSET, essence + (BUNDLE_NONCE_OFFSET - TRANSACTION_ESSENCE_OFFSET), BUNDLE_NONCE_LENGTH);

        uint64_t now;
#ifdef PICO_ON_DEVICE
        now = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
        now = emscripten_get_now();
#endif
        printf(" %llu tries %llums\n", number_of_tries, now - start);
    }

    void set_message_or_signature(const int8_t *signature, const int8_t security_level, const size_t offset) {

        for (int8_t i = 0; i < security_level; i++) {

            memcpy(transactions[offset + i] + MESSAGE_OR_SIGNATURE_OFFSET, signature + (i * MESSAGE_OR_SIGNATURE_LENGTH), MESSAGE_OR_SIGNATURE_LENGTH);
        }
    }

    void set_tag(const int8_t *tag, const size_t offset) {

        memcpy(transactions[offset] + TAG_OFFSET, tag, TAG_LENGTH);
    }

    void entangle(const int8_t *trunk_transaction, const int8_t *branch_transaction, const int8_t security_level, int8_t *transactions_copy, int8_t *digests) {
        for (ssize_t index = (ssize_t)(length - 1); index >= 0; index--) {
    
            printf("Searching tx nonce...");

            uint64_t start;
#ifdef PICO_ON_DEVICE
            start = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
            start = emscripten_get_now();
#endif
    
            uint64_t number_of_tries = 1;

            memcpy(transactions[index] + TRUNK_TRANSACTION_OFFSET,  (index == (length - 1)) ? trunk_transaction : digests + ((index + 1) * HASH_LENGTH), HASH_LENGTH);
            memcpy(transactions[index] + BRANCH_TRANSACTION_OFFSET, branch_transaction, HASH_LENGTH);

            curl->get_digest(transactions[index], 0, TRANSACTION_LENGTH, digests, index * HASH_LENGTH);

            const int8_t type_trit = (transaction_types[index] == ATTACHMENT_VALIDATION) ? 0
                                   : (transaction_types[index] == BUNDLE_VALIDATION ? 1 : -1);
            const int8_t head_flag_trit = (index == (length - 1)) ? 1 : -1;
            const int8_t tail_flag_trit = (index == 0) ? 1 : -1;

            while (
                !(
                    ((index > 0) || verify_bundle(digests + index * HASH_LENGTH, security_level)) &&
                    digests[index * HASH_LENGTH] == type_trit &&
                    digests[index * HASH_LENGTH + 1] == head_flag_trit && 
                    digests[index * HASH_LENGTH + 2] == tail_flag_trit
                )
            ) {
                // mini-PoW to find digest flags

                for (size_t i = 0; i < NONCE_LENGTH; i++) {

                    int8_t &nonce = transactions[index][NONCE_OFFSET + i];
                    nonce++;

                    if (nonce > 1) {

                        nonce = -1;
                    } else {

                        break;
                    }
                }

                curl->get_digest(transactions[index], 0, TRANSACTION_LENGTH, digests, index * HASH_LENGTH);
                number_of_tries++;
            };

            memcpy(transactions_copy + (index * TRANSACTION_LENGTH), transactions[index], TRANSACTION_LENGTH);
            
            uint64_t now;
#ifdef PICO_ON_DEVICE
            now = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
            now = emscripten_get_now();
#endif
            printf(" %llu tries %llums", number_of_tries, now - start);

            Transaction transaction;
            const bool transaction_validity = transaction.copy(curl, transactions[index]);

            printf(transaction_validity ? "valid" : "invalid");

            if (transaction_validity) {

                tangle->put(&transaction);
            }

            printf("\n");

            transmitter->transmit(false);

#ifdef __EMSCRIPTEN__
            EM_ASM({
                if (Module.onEntanglement) {
                    Module.onEntanglement($0, $1, $2);
                }
            }, digests, transactions_copy, index);
#endif
        }

        length = 0;
    }
};
