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

template <size_t L>
struct Receiver {

    Curl_729_27 *curl;
    Transaction transaction;
    Tangle *tangle;

    uint64_t number_of_received_bytes = 0;
    uint64_t number_of_received_transactions = 0;
    uint64_t number_of_new_transactions = 0;
    uint64_t number_of_dropped_packets = 0;

    size_t (*receive_frame)(uint8_t*, size_t);

    void receive_packet() {

        size_t index = 0;
        uint8_t frame[TRANSACTION_BYTE_LENGTH];
        uint8_t packet[TRANSACTION_BYTE_LENGTH];
        int8_t trits[TRANSACTION_LENGTH] = {0};

        uint64_t start;
#ifdef PICO_ON_DEVICE
        start = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
        start = emscripten_get_now();
#endif
        size_t number_of_received_bytes = 0;
        size_t number_of_latest_received_bytes;

        do {

            number_of_latest_received_bytes = receive_frame(packet + (index * L), L);
            number_of_received_bytes += number_of_latest_received_bytes;

            uint64_t now;
#ifdef PICO_ON_DEVICE
            now = to_ms_since_boot(get_absolute_time());

#elif defined(__EMSCRIPTEN__)
            now = emscripten_get_now();
#endif

            if ((now - start) > (10 * 1000)) {
    
                memset(packet, 0, index * L);
                index = 0;
                return;
            }
        } while(number_of_received_bytes < TRANSACTION_BYTE_LENGTH || ((((uint16_t)packet[index * L] << 8) | packet[index * L + 1]) != PACKET_END));

        size_t payload_byte_length = (index - 1) * L;

        memset(packet, 0, index * L);
        index = 0;

        bytes_to_trits(packet, payload_byte_length, trits);

        if (transaction.copy(curl, trits)) { // valid tx

            number_of_received_transactions++;

#ifdef PICO_ON_DEVICE
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            sleep_ms(100);
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
#endif

            if (tangle->put(&transaction)) { // new tx

                number_of_new_transactions++;

#ifdef __EMSCRIPTEN__
                int8_t digest_trits[HASH_LENGTH];

                bytes_to_trits(transaction.digest, HASH_BYTE_LENGTH, digest_trits);

                EM_ASM({

                    if (Module.onReceivedNewTransaction) {

                        Module.onReceivedNewTransaction($0, $0);
                    }
                }, trits, digest_trits);
#endif
            }

#ifdef __EMSCRIPTEN__
            int8_t digest_trits[HASH_LENGTH];

            bytes_to_trits(transaction.digest, HASH_BYTE_LENGTH, digest_trits);

            EM_ASM({

                if (Module.onReceivedTransaction) {

                    Module.onReceivedTransaction($0, $0);
                }
            }, trits, transaction.digest);
#endif
        } else { // invalid tx

            number_of_dropped_packets++;

#ifdef __EMSCRIPTEN__
            EM_ASM({

                if (Module.onInvalidTransaction) {

                    Module.onInvalidTransaction($0);
                }
            }, trits);
#endif
        }
    }
};
