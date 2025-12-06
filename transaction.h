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

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "curl.h"
#include "converter.h"

#define HASH_BYTE_LENGTH                    BYTE_LENGTH(HASH_LENGTH)

#define TYPE_OFFSET                         0
#define HEAD_FLAG_OFFSET                    1
#define TAIL_FLAG_OFFSET                    2


#define MESSAGE_OR_SIGNATURE_OFFSET         0
#define MESSAGE_OR_SIGNATURE_LENGTH         (((HASH_LENGTH / 3) / 3) * HASH_LENGTH)  // 6561
#define MESSAGE_OR_SIGNATURE_BYTE_OFFSET    BYTE_LENGTH(MESSAGE_OR_SIGNATURE_OFFSET)
#define MESSAGE_OR_SIGNATURE_BYTE_LENGTH    BYTE_LENGTH(MESSAGE_OR_SIGNATURE_LENGTH)


#define EXTRA_DATA_DIGEST_OFFSET            (MESSAGE_OR_SIGNATURE_OFFSET + MESSAGE_OR_SIGNATURE_LENGTH)
#define EXTRA_DATA_DIGEST_BYTE_OFFSET       BYTE_LENGTH(EXTRA_DATA_DIGEST_OFFSET)

#define ADDRESS_OFFSET                      (EXTRA_DATA_DIGEST_OFFSET + HASH_LENGTH)
#define ADDRESS_BYTE_OFFSET                 BYTE_LENGTH(ADDRESS_OFFSET)

#define ENERGY_OFFSET                       (ADDRESS_OFFSET + HASH_LENGTH)
#define ENERGY_LENGTH                       81
#define ENERGY_BYTE_OFFSET                  BYTE_LENGTH(ENERGY_OFFSET)
#define ENERGY_BYTE_LENGTH                  BYTE_LENGTH(ENERGY_BYTE_LENGTH)

#define ISSUANCE_TIMESTAMP_OFFSET           (ENERGY_OFFSET + ENERGY_LENGTH)
#define ISSUANCE_TIMESTAMP_LENGTH           27
#define ISSUANCE_TIMESTAMP_BYTE_OFFSET      BYTE_LENGTH(ISSUANCE_TIMESTAP_OFFSET)
#define ISSUANCE_TIMESTAMP_BYTE_LENGTH      BYTE_LENGTH(ISSUANCE_TIMESTAP_LENGTH)

#define TIMELOCK_LOWER_BOUND_OFFSET         (ISSUANCE_TIMESTAMP_OFFSET + ISSUANCE_TIMESTAMP_LENGTH)
#define TIMELOCK_LOWER_BOUND_LENGTH         27
#define TIMELOCK_LOWER_BOUND_BYTE_OFFSET    BYTE_LENGTH(TIMELOCK_LOWER_BOUND_OFFSET)
#define TIMELOCK_LOWER_BOUND_BYTE_LENGTH    BYTE_LENGTH(TIMELOCK_LOWER_BOUND_LENGTH)

#define TIMELOCK_UPPER_BOUND_OFFSET         (TIMELOCK_LOWER_BOUND_OFFSET + TIMELOCK_LOWER_BOUND_LENGTH)
#define TIMELOCK_UPPER_BOUND_LENGTH         27
#define TIMELOCK_UPPER_BOUND_BYTE_OFFSET    BYTE_LENGTH(TIMELOCK_UPPER_BOUND_OFFSET)
#define TIMELOCK_UPPER_BOUND_BYTE_LENGTH    BYTE_LENGTH(TIMELOCK_UPPER_BOUND_LENGTH)

#define BUNDLE_NONCE_OFFSET                 (TIMELOCK_UPPER_BOUND_OFFSET + TIMELOCK_UPPER_BOUND_LENGTH)
#define BUNDLE_NONCE_LENGTH                 81
#define BUNDLE_NONCE_BYTE_OFFSET            BYTE_LENGTH(BUNDLE_NONCE_OFFSET)
#define BUNDLE_NONCE_BYTE_LENGTH            BYTE_LENGTH(BUNDLE_NONCE_LENGTH)


#define TRUNK_TRANSACTION_OFFSET            (BUNDLE_NONCE_OFFSET + BUNDLE_NONCE_LENGTH)
#define TRUNK_TRANSACTION_BYTE_OFFSET       BYTE_LENGTH(TRUNK_TRANSACTION_OFFSET)
#define BRANCH_TRANSACTION_OFFSET           (TRUNK_TRANSACTION_OFFSET + HASH_LENGTH)
#define BRANCH_TRANSACTION_BYTE_OFFSET      BYTE_LENGTH(BRANCH_TRANSACTION_OFFSET)

#define TAG_OFFSET                          (BRANCH_TRANSACTION_OFFSET + HASH_LENGTH)
#define TAG_LENGTH                          81
#define TAG_BYTE_OFFSET                     BYTE_LENGTH(TAG_OFFSET)
#define TAG_BYTE_LENGTH                     BYTE_LENGTH(TAG_LENGTH)

#define ATTACHMENT_TIMESTAMP_OFFSET         (TAG_OFFSET + TAG_LENGTH)
#define ATTACHMENT_TIMESTAMP_LENGTH         27
#define ATTACHMENT_TIMESTAMP_BYTE_OFFSET    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_OFFSET)
#define ATTACHMENT_TIMESTAMP_BYTE_LENGTH    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_LENGTH)

#define ATTACHMENT_TIMESTAMP_LOWER_BOUND_OFFSET         (ATTACHMENT_TIMESTAMP_OFFSET + ATTACHMENT_TIMESTAMP_LENGTH)
#define ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH         27
#define ATTACHMENT_TIMESTAMP_LOWER_BOUND_BYTE_OFFSET    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_LOWER_BOUND_OFFSET)
#define ATTACHMENT_TIMESTAMP_LOWER_BOUND_BYTE_LENGTH    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH)

#define ATTACHMENT_TIMESTAMP_UPPER_BOUND_OFFSET         (ATTACHMENT_TIMESTAMP_LOWER_BOUND_OFFSET + ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH)
#define ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH         27
#define ATTACHMENT_TIMESTAMP_UPPER_BOUND_BYTE_OFFSET    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_UPPER_BOUND_OFFSET)
#define ATTACHMENT_TIMESTAMP_UPPER_BOUND_BYTE_LENGTH    BYTE_LENGTH(ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH)

#define NONCE_OFFSET                        (ATTACHMENT_TIMESTAMP_UPPER_BOUND_OFFSET + ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH)
#define NONCE_LENGTH                        81
#define NONCE_BYTE_OFFSET                   BYTE_LENGTH(NONCE_OFFSET)
#define NONCE_BYTE_LENGTH                   BYTE_LENGTH(NONCE_LENGTH)


#define TRANSACTION_ESSENCE_OFFSET          EXTRA_DATA_DIGEST_OFFSET
#define TRANSACTION_ESSENCE_LENGTH          (TRUNK_TRANSACTION_OFFSET - TRANSACTION_ESSENCE_OFFSET)
#define TRANSACTION_ESSENCE_BYTE_OFFSET     BYTE_LENGHT(TRANSACTION_ESSENCE_OFFSET)
#define TRANSACTION_ESSENCE_BYTE_LENGTH     BYTE_LENGTH(TRANSACTION_ESSENCE_LENGTH)

#define TRANSACTION_LENGTH                  (NONCE_OFFSET + NONCE_LENGTH)
#define TRANSACTION_BYTE_LENGTH             BYTE_LENGTH(TRANSACTION_LENGTH)

struct Transaction {

    uint8_t digest[HASH_LENGTH];

    uint8_t message_or_signature[MESSAGE_OR_SIGNATURE_BYTE_LENGTH];
    uint8_t extra_data_digest[HASH_BYTE_LENGTH];
    uint8_t address[HASH_BYTE_LENGTH];
    int64_t energy;
    int64_t timelock_lower_bound;
    int64_t timelock_upper_bound;
    uint8_t bundle_nonce[BUNDLE_NONCE_BYTE_LENGTH];

    uint8_t trunk_transaction[HASH_BYTE_LENGTH];
    uint8_t branch_transaction[HASH_BYTE_LENGTH];
    uint8_t tag[TAG_BYTE_LENGTH];
    int64_t attachment_timestamp;
    int64_t attachment_timestamp_lower_bound;
    int64_t attachment_timestamp_upper_bound;
    uint8_t nonce[NONCE_BYTE_LENGTH];

    bool copy(Curl_729_27* curl, int8_t *trits) {

        uint8_t bytes[TRANSACTION_BYTE_LENGTH];
        memset(bytes, 0, TRANSACTION_BYTE_LENGTH);
        trits_to_bytes(trits, TRANSACTION_LENGTH, bytes);

        if (trits[BRANCH_TRANSACTION_OFFSET + TAIL_FLAG_OFFSET] != 1) { // branch transaction must be tail

            return false;
        }

        attachment_timestamp = trits_to_int(trits + ATTACHMENT_TIMESTAMP_OFFSET, ATTACHMENT_TIMESTAMP_LENGTH);
        attachment_timestamp_lower_bound = trits_to_int(trits + ATTACHMENT_TIMESTAMP_LOWER_BOUND_OFFSET, ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH);

        if (attachment_timestamp < attachment_timestamp_lower_bound) {

            return false;
        }

        attachment_timestamp_upper_bound = trits_to_int(trits + ATTACHMENT_TIMESTAMP_UPPER_BOUND_OFFSET, ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH);
        if (attachment_timestamp > attachment_timestamp_upper_bound) {

            return false;
        }

        timelock_lower_bound = trits_to_int(trits + TIMELOCK_LOWER_BOUND_OFFSET, TIMELOCK_LOWER_BOUND_LENGTH);
        timelock_upper_bound = trits_to_int(trits + TIMELOCK_UPPER_BOUND_OFFSET, TIMELOCK_UPPER_BOUND_LENGTH);

        if (timelock_lower_bound > timelock_upper_bound) {

            return false;
        }

        if ((timelock_lower_bound != 0 && attachment_timestamp_lower_bound < timelock_upper_bound) ||
            (timelock_upper_bound != 0 && attachment_timestamp_upper_bound > timelock_upper_bound)) {

            return false;
        }

        int8_t digest_trits[HASH_LENGTH]; 
        curl->get_digest(trits, 0, TRANSACTION_LENGTH, digest_trits, 0);

        if (digest_trits[HEAD_FLAG_OFFSET] == 0 || digest_trits[TAIL_FLAG_OFFSET] == 0) {

            return false;
        }

        // Trunk transaction of a head must be tail
        if (digest_trits[HEAD_FLAG_OFFSET] == 1 && trits[TRUNK_TRANSACTION_OFFSET + TAIL_FLAG_OFFSET] != 1) {

            return false;
        }

        // Trunk transaction of a non-head must not be tail
        if (digest_trits[HEAD_FLAG_OFFSET] == -1 && trits[TRUNK_TRANSACTION_OFFSET + TAIL_FLAG_OFFSET] == 1) {

            return false;
        }

        trits_to_bytes(digest_trits, HASH_LENGTH, digest);

        memcpy(message_or_signature, bytes + MESSAGE_OR_SIGNATURE_BYTE_OFFSET, MESSAGE_OR_SIGNATURE_BYTE_LENGTH);

        memcpy(extra_data_digest, bytes + EXTRA_DATA_DIGEST_BYTE_OFFSET, HASH_BYTE_LENGTH);
        memcpy(address, bytes + ADDRESS_BYTE_OFFSET, HASH_BYTE_LENGTH);
        energy = trits_to_int(trits + ENERGY_OFFSET, ENERGY_LENGTH);
        memcpy(bundle_nonce, bytes + BUNDLE_NONCE_BYTE_OFFSET, BUNDLE_NONCE_BYTE_LENGTH);

        memcpy(trunk_transaction, bytes + TRUNK_TRANSACTION_BYTE_OFFSET, HASH_BYTE_LENGTH);
        memcpy(branch_transaction, bytes + BRANCH_TRANSACTION_BYTE_OFFSET, HASH_BYTE_LENGTH);
        memcpy(tag, bytes + TAG_BYTE_OFFSET, TAG_BYTE_LENGTH);
        memcpy(nonce, bytes + NONCE_BYTE_OFFSET, NONCE_BYTE_LENGTH);

        return true;
    }


    void dump(uint8_t *bytes) {

        int8_t integer_trits[81] = { 0 };

        memcpy(bytes + MESSAGE_OR_SIGNATURE_BYTE_OFFSET, message_or_signature, MESSAGE_OR_SIGNATURE_BYTE_LENGTH);

        memcpy(bytes + EXTRA_DATA_DIGEST_BYTE_OFFSET, extra_data_digest, HASH_BYTE_LENGTH);
        memcpy(bytes + ADDRESS_BYTE_OFFSET, address, HASH_BYTE_LENGTH);
        int_to_trits(energy, integer_trits, 0, ENERGY_LENGTH);
        trits_to_bytes(integer_trits, ENERGY_LENGTH, bytes + ENERGY_BYTE_OFFSET);
        memset(integer_trits, 0, 81);
        int_to_trits(timelock_lower_bound, integer_trits, 0, TIMELOCK_LOWER_BOUND_LENGTH);
        trits_to_bytes(integer_trits, TIMELOCK_LOWER_BOUND_LENGTH, bytes + TIMELOCK_LOWER_BOUND_BYTE_OFFSET);
        memset(integer_trits, 0, 81);
        int_to_trits(timelock_upper_bound, integer_trits, 0, TIMELOCK_UPPER_BOUND_LENGTH);
        trits_to_bytes(integer_trits, TIMELOCK_UPPER_BOUND_LENGTH, bytes + TIMELOCK_UPPER_BOUND_BYTE_OFFSET);
        memcpy(bytes + BUNDLE_NONCE_BYTE_OFFSET, bundle_nonce, BUNDLE_NONCE_BYTE_LENGTH);

        memcpy(bytes + TRUNK_TRANSACTION_BYTE_OFFSET, trunk_transaction, HASH_BYTE_LENGTH);
        memcpy(bytes + BRANCH_TRANSACTION_BYTE_OFFSET, branch_transaction, HASH_BYTE_LENGTH);
        memcpy(bytes + TAG_BYTE_OFFSET, tag, TAG_BYTE_LENGTH);
        memset(integer_trits, 0, 81);
        int_to_trits(attachment_timestamp, integer_trits, 0, ATTACHMENT_TIMESTAMP_LENGTH);
        trits_to_bytes(integer_trits, ATTACHMENT_TIMESTAMP_LENGTH, bytes + ATTACHMENT_TIMESTAMP_BYTE_OFFSET);
        memset(integer_trits, 0, 81);
        int_to_trits(attachment_timestamp_lower_bound, integer_trits, 0, ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH);
        trits_to_bytes(integer_trits, ATTACHMENT_TIMESTAMP_LOWER_BOUND_LENGTH, bytes + ATTACHMENT_TIMESTAMP_LOWER_BOUND_BYTE_OFFSET);
        memset(integer_trits, 0, 81);
        int_to_trits(attachment_timestamp_upper_bound, integer_trits, 0, ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH);
        trits_to_bytes(integer_trits, ATTACHMENT_TIMESTAMP_UPPER_BOUND_LENGTH, bytes + ATTACHMENT_TIMESTAMP_UPPER_BOUND_BYTE_OFFSET);
        memcpy(bytes + NONCE_BYTE_OFFSET, nonce, NONCE_BYTE_LENGTH);
    }
};

#endif
