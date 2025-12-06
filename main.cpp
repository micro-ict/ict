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

#include <stdio.h>
#include <cstring>
#include <cinttypes>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/structs/xip.h"

#include "ict.h"

// LED
#define PICO_DEFAULT_LED_PIN    25
#define LED_DELAY_MS            300

// Use the CS pin your board wires PSRAM to (Pico Plus 2 commonly uses GPIO47)
#define PSRAM_CS_PIN 47
// Use the cached mapped base reported by community: 0x11000000
#define PSRAM_BASE ((uint8_t *)0x11000000)
#define PSRAM_SIZE (8 * 1024 * 1024)

void enable_psram_xip(void) {
    // route pin to XIP CS1 (alternate function F9 on RP2350 in SDK)
    gpio_set_function(PSRAM_CS_PIN, GPIO_FUNC_XIP_CS1);

    // make XIP memory region M1 writable (enables cached R/W at 0x11000000)
    xip_ctrl_hw->ctrl |= XIP_CTRL_WRITABLE_M1_BITS;

    sleep_ms(1);
}

int main() {

    stdio_init_all();

    enable_psram_xip();

    sleep_ms(1000); // allow USB to enumerate

    IcT ict = IcT();

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    int64_t latest_broadcast = 0;

    while (true) {
        
        ict.receiver.receive_packet();

        int64_t now = to_ms_since_boot(get_absolute_time());

        if (now - latest_broadcast > 1000 * 10) { // test

            int8_t transaction[TRANSACTION_LENGTH] = { 0 };
            int8_t transactions_copy[MAX_OWN_BUNDLE_SIZE * TRANSACTION_LENGTH];
            int8_t bundle[HASH_LENGTH];
            int8_t digests_copy[MAX_OWN_BUNDLE_SIZE * HASH_LENGTH];
            int8_t ref[HASH_LENGTH];
            TransactionDescription transaction_description;
    
            memset(transactions_copy, 0, MAX_OWN_BUNDLE_SIZE * TRANSACTION_LENGTH);
            memset(digests_copy, 0, MAX_OWN_BUNDLE_SIZE * HASH_LENGTH);
            memset(transaction_description.extra_data_digest, 0, HASH_LENGTH);
            memset(transaction_description.address, 0, HASH_LENGTH);
            transaction_description.energy = (int64_t)(0);
            transaction_description.issuance_timestamp = (int64_t)(0);
            transaction_description.timelock_lower_bound = (int64_t)(0);
            transaction_description.timelock_upper_bound = (int64_t)(0);

            ict.ixi.bundle_transaction(&transaction_description, ATTACHMENT_VALIDATION);
            ict.ixi.finalize_bundle(1, bundle);
        
            memset(ref, 0, HASH_LENGTH);
            ref[0] = 0;
            ref[1] = 1;
            ref[2] = 1;
    
            ict.ixi.entangle(ref, ref, 0, transactions_copy, digests_copy);

            latest_broadcast = now;
        }

        sleep_ms(10);
    }

    return 0;
}