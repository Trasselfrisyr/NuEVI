/*  Copyright © 2007 Apple Inc. All Rights Reserved.

Disclaimer: IMPORTANT:  This Apple software is supplied to you by 
        Apple Inc. ("Apple") in consideration of your agreement to the
        following terms, and your use, installation, modification or
        redistribution of this Apple software constitutes acceptance of these
        terms.  If you do not agree with these terms, please do not use,
        install, modify or redistribute this Apple software.

        In consideration of your agreement to abide by the following terms, and
        subject to these terms, Apple grants you a personal, non-exclusive
        license, under Apple's copyrights in this original Apple software (the
        "Apple Software"), to use, reproduce, modify and redistribute the Apple
        Software, with or without modifications, in source and/or binary forms;
        provided that if you redistribute the Apple Software in its entirety and
        without modifications, you must retain this notice and the following
        text and disclaimers in all such redistributions of the Apple Software. 
        Neither the name, trademarks, service marks or logos of Apple Inc. 
        may be used to endorse or promote products derived from the Apple
        Software without specific prior written permission from Apple.  Except
        as expressly stated in this notice, no other rights or licenses, express
        or implied, are granted by Apple herein, including but not limited to
        any patent rights that may be infringed by your derivative works or by
        other works in which the Apple Software may be incorporated.

        The Apple Software is provided by Apple on an "AS IS" basis.  APPLE
        MAKES NO WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION
        THE IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY AND FITNESS
        FOR A PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND
        OPERATION ALONE OR IN COMBINATION WITH YOUR PRODUCTS.

        IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL
        OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
        SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
        INTERRUPTION) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION,
        MODIFICATION AND/OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED
        AND WHETHER UNDER THEORY OF CONTRACT, TORT (INCLUDING NEGLIGENCE),
        STRICT LIABILITY OR OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE
        POSSIBILITY OF SUCH DAMAGE.
*/
#include <CoreMIDI/MIDIServices.h>
#include <CoreFoundation/CFRunLoop.h>
#include <stdio.h>

// ___________________________________________________________________________________________
// test program to echo MIDI In to Out
// ___________________________________________________________________________________________

MIDIPortRef     gOutPort = NULL;
MIDIEndpointRef gDest = NULL;
int             gChannel = 0;

static void MyReadProc(const MIDIPacketList *pktlist, void *refCon, void *connRefCon)
{
    if (gOutPort != NULL && gDest != NULL) {
        MIDIPacket *packet = (MIDIPacket *)pktlist->packet; // remove const (!)
        for (unsigned int j = 0; j < pktlist->numPackets; ++j) {
            for (int i = 0; i < packet->length; ++i) {
//              printf("%02X ", packet->data[i]);

                // rechannelize status bytes
                if (packet->data[i] >= 0x80 && packet->data[i] < 0xF0)
                    packet->data[i] = (packet->data[i] & 0xF0) | gChannel;
            }

//          printf("\n");
            packet = MIDIPacketNext(packet);
        }

        MIDISend(gOutPort, gDest, pktlist);
    }
}

int     main(int argc, char *argv[])
{
    if (argc >= 2) {
        // first argument, if present, is the MIDI channel number to echo to (1-16)
        sscanf(argv[1], "%d", &gChannel);
        if (gChannel < 1) gChannel = 1;
        else if (gChannel > 16) gChannel = 16;
        --gChannel; // convert to 0-15
    }

    // create client and ports
    MIDIClientRef client = NULL;
    MIDIClientCreate(CFSTR("MIDI Ech2"), NULL, NULL, &client);

    MIDIPortRef inPort = NULL;
    MIDIInputPortCreate(client, CFSTR("Input port"), MyReadProc, NULL, &inPort);
    MIDIOutputPortCreate(client, CFSTR("Output port"), &gOutPort);

    // enumerate devices (not really related to purpose of the echo program
    // but shows how to get information about devices)
    int i, n;
    CFStringRef pname, pmanuf, pmodel;
    char name[64], manuf[64], model[64];

    n = MIDIGetNumberOfDevices();
    for (i = 0; i < n; ++i) {
        MIDIDeviceRef dev = MIDIGetDevice(i);

        MIDIObjectGetStringProperty(dev, kMIDIPropertyName, &pname);
        MIDIObjectGetStringProperty(dev, kMIDIPropertyManufacturer, &pmanuf);
        MIDIObjectGetStringProperty(dev, kMIDIPropertyModel, &pmodel);

        CFStringGetCString(pname, name, sizeof(name), 0);
        CFStringGetCString(pmanuf, manuf, sizeof(manuf), 0);
        CFStringGetCString(pmodel, model, sizeof(model), 0);
        CFRelease(pname);
        CFRelease(pmanuf);
        CFRelease(pmodel);

        printf("name=%s, manuf=%s, model=%s\n", name, manuf, model);
    }

    // open connections from all sources
    n = MIDIGetNumberOfSources();
    printf("%d sources\n", n);
    for (i = 0; i < n; ++i) {
        MIDIEndpointRef src = MIDIGetSource(i);
        MIDIPortConnectSource(inPort, src, NULL);
    }

    // find the first destination
    n = MIDIGetNumberOfDestinations();
    if (n > 0)
        gDest = MIDIGetDestination(0);

    if (gDest != NULL) {
        MIDIObjectGetStringProperty(gDest, kMIDIPropertyName, &pname);
        CFStringGetCString(pname, name, sizeof(name), 0);
        CFRelease(pname);
        printf("Echoing to channel %d of %s\n", gChannel + 1, name);
    } else {
        printf("No MIDI destinations present\n");
    }

    CFRunLoopRun();
    // run until aborted with control-C

    return 0;
}