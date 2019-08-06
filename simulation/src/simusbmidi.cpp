#include <cstdint>
#include <cstdio>
#include <iostream>
#include <fstream>

#include <CoreFoundation/CFRunLoop.h>

#include "simusbmidi.h"

/*************************************
 *	Stub simulation of Teensy usbMidi
 */

void SimUsbMidi::sendNoteOff(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::noteOff] note %03d vel %03d ch %02d\n", note, velocity, channel);
}

void SimUsbMidi::sendNoteOn(uint8_t note, uint8_t velocity, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::noteOn] note %03d vel %03d ch %02d\n", note, velocity, channel);
}

void SimUsbMidi::sendPolyPressure(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::polyPressure] note %03d p %03d ch %02d\n", note, pressure, channel);
}

void SimUsbMidi::sendAfterTouchPoly(uint8_t note, uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::afterTouchPoly] note %03d p %03d ch %02d\n", note, pressure, channel);
}

void SimUsbMidi::sendControlChange(uint8_t control, uint8_t value, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::controlChange] cc %03d val %03d ch %02d\n", control, value, channel);
}

void SimUsbMidi::sendProgramChange(uint8_t program, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::programChange] prg %03d ch %02d\n", program, channel);
}

void SimUsbMidi::sendAfterTouch(uint8_t pressure, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::afterTouch] p %03d ch %02d\n", pressure, channel);
}

void SimUsbMidi::sendPitchBend(int value, uint8_t channel, uint8_t __unused cable)
{
	printf( "[usbMIDI::pitchBend] pb %05d ch %02d\n", value, channel);
}

void SimUsbMidi::sendSysEx(uint16_t length, const uint8_t __unused *data, bool __unused hasTerm, uint8_t __unused cable)
{
	printf( "[usbMIDI::sysEx] len %d\n", length);
	//this->sendRealMidi(data, length);

}

//Set a low chunk size on purpose just to let the receiver work for it
#define MIDI_SYSEX_CHUNK_SIZE 32

void SimUsbMidi::dumpa() {
	printf("[SimUsbMidi::read] Attempting to send midi data\n");

	std::ifstream file(this->midiFile, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	uint8_t *buffer = (uint8_t*)malloc(size);

	if (file.read((char*)buffer, size))
	{
		printf("[SimUsbMidi::dumpa] Sending %lu bytes.\n", size);

		this->sendRealMidi(buffer, size);
		//this->receiveMidiData(buffer, size);

	}
	free(buffer);
}

bool SimUsbMidi::read(uint8_t __unused channel) {
	if(this->sendMidi) {

		printf("[SimUsbMidi::read] Attempting to send midi data\n");

		std::ifstream file(this->midiFile, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		uint8_t *buffer = (uint8_t*)malloc(size);

		if (file.read((char*)buffer, size))
		{
			printf("[SimUsbMidi::read] Sending %lu bytes.\n", size);

			this->receiveMidiData(buffer, size);

		}
		free(buffer);

		this->sendMidi = false;

	}

	return false;
}


//Provide midi data for simulation to receive
void SimUsbMidi::receiveMidiData(const uint8_t *data, const uint16_t length) {
	if(length==0) return; //There is no data, what's even the point
	uint8_t midi_message = data[0]; //First byte of data

	if(midi_message != 0xF0) return; //Only sysex data supported (no other handlers available)

	if(this->usb_midi_handleSysExPartial) {
		//Chunked sysex receiver set, use that.
		if(length<=MIDI_SYSEX_CHUNK_SIZE) {
			//Send all in one go
			printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExPartial(complete) %d B\n", length);
			(this->usb_midi_handleSysExPartial)(data, length, true);
		} else {
			uint8_t* buf = (uint8_t*)malloc(MIDI_SYSEX_CHUNK_SIZE);
			int pos=0;
			while(pos<length) {
				int remaining = length-pos;
				int bytesToSend = std::min(remaining, MIDI_SYSEX_CHUNK_SIZE);
				bool complete = (bytesToSend == remaining);

				memcpy(buf, data+pos, bytesToSend);
				printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExPartial(complete: %d) %d B\n", complete, bytesToSend);
				(this->usb_midi_handleSysExPartial)(buf, bytesToSend, complete);
				pos=pos+bytesToSend;
			}
			free(buf);
		}

	} else if(this->usb_midi_handleSysExComplete) {
		printf( "[SimUsbMidi::receiveMidiData] usb_midi_handleSysExComplete() %d B\n", length);
		(this->usb_midi_handleSysExComplete)(data, length);
	} else {
		//Nobody listening
	}
}

//MIDI SysEx handlers. Choice of data types is a bit odd, but done to match Arduino/Teensy libraries
void SimUsbMidi::setHandleSystemExclusive(void (*fptr) (const uint8_t *array, unsigned int size)) {
	this->usb_midi_handleSysExComplete = fptr;
}

//"Chunked" sysex handler (teensy extension), for large messages
void SimUsbMidi::setHandleSystemExclusive(void (*fptr) (const uint8_t *array, uint16_t size, bool last)) {
	this->usb_midi_handleSysExPartial = (void (*)(const uint8_t *, uint16_t, uint8_t))fptr;
}

void SimUsbMidi::setMidiFile(std::string filename) {
	this->midiFile = filename;
}

void SimUsbMidi::triggerMidi() {
    this->sendMidi = true;
}

//Send midi data, only as a single packet
void SimUsbMidi::sendRealMidi(const uint8_t* message, uint16_t size) {

	printf("[SimUsbMidi::sendRealMidi] sending %d bytes midi data\n", size);

	if(!this->midiOutPort) {
		printf("[SimUsbMidi::sendRealMidi] no midi out port\n");
		return;
	}

	if (size==0) {
		printf("[SimUsbMidi::sendRealMidi] no data to send\n");
		return;
	}

  unsigned int nBytes = static_cast<unsigned int> (size);

  Byte buffer[nBytes+(sizeof( MIDIPacketList ))];
  ByteCount listSize = sizeof( buffer );
  MIDIPacketList *packetList = (MIDIPacketList*)buffer;
  MIDIPacket *packet = MIDIPacketListInit( packetList );

  ByteCount remainingBytes = nBytes;
  while ( remainingBytes && packet ) {
    ByteCount bytesForPacket = remainingBytes > 65535 ? 65535 : remainingBytes; // 65535 = maximum size of a MIDIPacket
    const Byte* dataStartPtr = (const Byte *) &message[nBytes - remainingBytes];
    packet = MIDIPacketListAdd( packetList, listSize, packet, 0, bytesForPacket, dataStartPtr );
    remainingBytes -= bytesForPacket;
  }

  	OSStatus result;

    result = MIDISend(this->midiOutPort, this->midiDestination, packetList );
    if ( result != noErr ) {
		printf("[SimUsbMidi::sendRealMidi] error sending MIDI message: %u\n", result);
		return;
    }

	// Send to any destinations that may have connected to us.
    result = MIDIReceived( this->midiDestination, packetList );
	if ( result != noErr ) {
        printf("[SimUsbMidi::sendRealMidi] error sending MIDI to virtual destinations.\n");
	}

    printf("[SimUsbMidi::sendRealMidi] Success?\n");


}

static void midiInputHandler(const MIDIPacketList *pktlist, void *refCon, __unused void *connRefCon)
{
	SimUsbMidi *simUsbMidi = (SimUsbMidi*)refCon;
    //if (gOutPort != NULL && gDest != NULL) {
	if (simUsbMidi->midiOutPort && simUsbMidi->midiDestination) {
        MIDIPacket *packet = (MIDIPacket *)pktlist->packet; // remove const (!)
        for (unsigned int j = 0; j < pktlist->numPackets; ++j) {
            for (int i = 0; i < packet->length; ++i) {
//              printf("%02X ", packet->data[i]);

                // rechannelize status bytes
                if (packet->data[i] >= 0x80 && packet->data[i] < 0xF0)
                    packet->data[i] = (packet->data[i] & 0xF0) | simUsbMidi->midiChannel;
            }

//          printf("\n");
            packet = MIDIPacketNext(packet);
        }

        //MIDISend(gOutPort, gDest, pktlist);
    }
}


void SimUsbMidi::setupCoreMidi() {
	    // create client and ports
    //MIDIClientRef client = NULL;
    MIDIClientRef client = 0;

    CFStringRef clientName = CFSTR("NuEVI simulator");

    OSStatus result;
    result = MIDIClientCreate(clientName, NULL, NULL, &client);

	if (result != noErr) {
	    printf("Error creating MIDI client: %u", (int)result);
	    return;
	}

	result = MIDISourceCreate(client, clientName, &(this->midiOutPort));
	if (result != noErr) {
		printf("Error creating MIDI source: %u", (int)result);
		return;
	}

	result = MIDIDestinationCreate(client, clientName, midiInputHandler, this, &(this->midiInPort));
	if (result != noErr) {
	    printf("Error creating MIDI destination: %u", (int)result);
	    return;
	}

    printf("I am a MIDI device now!\n");

}
