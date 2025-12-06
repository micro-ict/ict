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

#ifndef TANGLE_CAPACITY
#define TANGLE_CAPACITY                 243
#endif

#ifndef MERKLE_TREE_DEPTH
#define MERKLE_TREE_DEPTH               7
#endif

// PACKET DISSEMINATION
//
// All received packets are put in special queue, a delay in `packet_reception_time_ms * [A, B]` range is chosen.
// Packet is broadcasted if was received just once during this timeframe.
//
#ifndef DISSEMINATION_ARGUMENT_ALPHA
#define DISSEMINATION_ARGUMENT_ALPHA    3
#endif

#ifndef DISSEMINATION_ARGUMENT_BETA
#define DISSEMINATION_ARGUMENT_BETA     9
#endif

#ifndef FRAME_LENGTH
#define FRAME_LENGTH                    32
#endif

#ifndef MAX_OWN_BUNDLE_SIZE
#define MAX_OWN_BUNDLE_SIZE             27
#endif

#include "iss.h"
#include "mss.h"
#include "tangle.h"
#include "receiver.h"
#include "transmitter.h"
#include "ixi.h"

static Receiver<FRAME_LENGTH>* active_receiver = NULL;

struct IcT {

    Curl_729_27 curl;
    Tangle tangle;
    Transmitter<FRAME_LENGTH> transmitter;
    Receiver<FRAME_LENGTH> receiver;

    ISS iss;
    MSS<MERKLE_TREE_DEPTH> mss;

    IxI ixi;

    IcT() {

        iss.curl = &curl;
        mss.iss = &iss;

        receiver.tangle = &tangle;
        receiver.curl = &curl;
        transmitter.tangle = &tangle;

        ixi.curl = &curl;
        ixi.tangle = &tangle;
        ixi.transmitter = &transmitter;

        printf("Iota Controlled agenT is luanching...\n");
    }

    void log() {

        printf(
            "[RX]: %llu, %llu/*%llu/-%llu [TX]: %llu, %llu\n",
            receiver.number_of_received_bytes,
            receiver.number_of_received_transactions,
            receiver.number_of_new_transactions,
            receiver.number_of_dropped_packets,
            transmitter.number_of_transmitted_bytes,
            transmitter.number_of_transmitted_transactions
        );
    }
};
