/*
 *  Copyright 2015 Martin Sidaway
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>

#include "myusb_atexit.h"
#include "myusb_utils.h"

#define DATA_BUFFER_LEN  27
#define TRANSFER_TIMEOUT 500
#define BITMAP_OFFSET    5
#define BITMAP_LEN       4
#define VELS_OFFSET      8
#define VELS_LEN         5
#define NUM_KEYS         (BITMAP_LEN * 8 - 7)
#define NORMAL_CHAN      0
#define DEFAULT_VEL      0x40
#define DRUMMAP_CHAN     9
#define DRUMMAP_NKEYS    12

#define OCTAVE_OFFSET    0
#define OCTAVE_DOWN      1 // Key "1"
#define OCTAVE_UP        4 // Key "B"
#define PATCH_OFFSET     0
#define PATCH_DOWN       2 // Key "A"
#define PATCH_UP         8 // Key "2"
#define PBBTN_OFFSET     13
#define PBBTN_VALUE      0x80
#define MOD_OFFSET       15

#define DPAD_OFFSET      2
#define DPAD_MASK        0xf
#define DPAD_UP          0
#define DPAD_RIGHT       2
#define DPAD_DOWN        4
#define DPAD_LEFT        6
#define DPAD_CENTER      8

#define MIDI_NOTEON      0x90
#define MIDI_NOTEOFF     0x80
#define MIDI_PROGCH      0xC0
#define MIDI_CTRLCH      0xB0
#define MIDI_PBEND       0xE0

#define MIDI_CHANMASK    0xf
#define MIDI_NUMPATCHES  0x80

uint8_t drumMapNotes[] = {
    35, 36, 38, 40, 41, 47,
    50, 42, 46, 49, 51, 53,
};

void mypm_terminate(void *ignored) {
    Pm_Terminate();
}

void mypm_close(void *data) {
    PortMidiStream *str = (PortMidiStream *)data;
    Pm_Close(str);
}

static inline int keyPressed(const char *curBuffer, const char *lastBuffer,
                             int index, int mask) {
    return (curBuffer[index] & mask) == mask && (lastBuffer[index] & mask) == 0;
}

int main(int argc, char **argv) {
    int r;
    int i;
    int printUsage = 0;

    if (argc < 2) {
        fprintf(stderr, "\nUsage: rb3_driver \"MIDI output device name\"\n");
        fprintf(stderr, "\nAvailable output devices:\n\n");
        printUsage = 1;
        //return 4;
    }

    /* initialize portmidi */
    r = Pm_Initialize();
    if (r < 0) {
        fprintf(stderr, "Failed to initialize portmidi\n");
        return r;
    };
    my_atexit(mypm_terminate, NULL);

    int pmdCount = Pm_CountDevices();
    //fprintf(stderr, "Got %d portmidi devices\n", pmdCount);
    PmDeviceID pmDev = -1;
    for (i = 0; i < pmdCount; i++) {
        const PmDeviceInfo *pmdInfo = Pm_GetDeviceInfo(i);
        if (pmdInfo != NULL && pmdInfo->output != 0) {
            if (printUsage) {
                fprintf(stderr, "  \"%s\"\n", pmdInfo->name);
            } else if (strcmp(argv[1], pmdInfo->name) == 0) {
                pmDev = i;
            }
        }
    }
    if (printUsage) {
        fprintf(stderr, "\n");
        return 4;
    }
    if (pmDev == -1) {
        fprintf(stderr, "Unable to find MIDI output device\n");
        return 3;
    }

    PortMidiStream *outStream;
    r = Pm_OpenOutput(&outStream, pmDev, NULL, 0, NULL, NULL, 0);
    if (r < 0) {
        fprintf(stderr, "Failed to open MIDI output device\n");
        return r;
    }
    my_atexit(mypm_close, outStream);

    fprintf(stderr, "Got MIDI output device!\n");

    /* Initialize libusb */
    r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to initialize libusb\n");
        return r;
    }
    my_atexit(myusb_exit, NULL);

    libusb_device *dev =
        myusb_get_device_by_prod_name_prefix("Harmonix RB3 Keyboard", 0);

    if (dev == NULL) {
        fprintf(stderr, "Failed to find input device\n");
        return 1;
    }

    fprintf(stderr, "Brilliant news! Found input device!\n");

    uint8_t interface_number = 0;
    const struct libusb_endpoint_descriptor *endpoint =
        myusb_get_endpoint(dev, LIBUSB_ENDPOINT_IN,
                     LIBUSB_TRANSFER_TYPE_MASK, LIBUSB_TRANSFER_TYPE_INTERRUPT, 0,
                     &interface_number);

    if (endpoint == NULL) {
        fprintf(stderr, "No suitable endpoint on input device\n");
        return 2;
    }

    fprintf(stderr, "Got input device endpoint!\n");

    libusb_device_handle *h = NULL;
    r = libusb_open(dev, &h);
    if (r < 0) {
        fprintf(stderr, "Failed to open input device\n");
        return r;
    }
    my_atexit(myusb_close, h);

    r = libusb_claim_interface(h, interface_number);
    if (r < 0) {
        fprintf(stderr, "Failed to claim input device interface\n");
        return r;
    }
    myusb_atexit_release_interface(h, interface_number);

    uint8_t buffer1[DATA_BUFFER_LEN];
    uint8_t buffer2[DATA_BUFFER_LEN];
    memset(buffer1, 0, sizeof(buffer1));
    memset(buffer2, 0, sizeof(buffer2));
    uint8_t *curBuffer = buffer1;
    uint8_t *lastBuffer = buffer2;
    int transferred_len = 0;
    int numKeysDown = 0;
    uint8_t notesDown[NUM_KEYS]; // What note was last activated for a given key?
    uint8_t chansDown[DRUMMAP_NKEYS]; // What channel was last activated for a given key?
    uint8_t firstNote = 48;
    int8_t curPatch = 0;
    int8_t drumMapOn = 0;
    
    //struct libusb_transfer transfer;
    //libusb_fill_interrupt_transfer(&transfer, h, endpoint->bEndpointAddress, buffer, DATA_BUFFER_LEN, got_data, NULL, TRANSFER_TIMEOUT);

    uint8_t bitmask;
    int cv, lv;
    while (1) {

        r = libusb_interrupt_transfer(h, endpoint->bEndpointAddress, curBuffer, DATA_BUFFER_LEN, &transferred_len, TRANSFER_TIMEOUT);

        if (r == LIBUSB_ERROR_TIMEOUT) {
            fprintf(stderr, "Data transfer timed out (input)\n");
            continue;
        }

        if (r < 0 || transferred_len == 0) {
            // N.B. this happens when the USB dongle is removed.
            fprintf(stderr, "Data transfer failed (input)\n");
            break;
        }

        if (transferred_len < DATA_BUFFER_LEN) {
            fprintf(stderr, "Wrong packet size (input)\n");
            continue;
        }

        if (memcmp(curBuffer, lastBuffer, DATA_BUFFER_LEN) != 0) {

            // DEBUG: dump input USB packet
            //for (i = 0; i < DATA_BUFFER_LEN; i++) {
            //    fprintf(stderr, " %02x", curBuffer[i]);
            //}
            //fprintf(stderr, "\n");

            if (curBuffer[OCTAVE_OFFSET] != lastBuffer[OCTAVE_OFFSET]) {

                if (keyPressed(curBuffer, lastBuffer, OCTAVE_OFFSET, OCTAVE_UP) &&
                    firstNote <= 84) {
                    firstNote += 12;
                }
                if (keyPressed(curBuffer, lastBuffer, OCTAVE_OFFSET, OCTAVE_DOWN) &&
                    firstNote >= 12) {
                    firstNote -= 12;
                }

                // N.B. assuming OCTAVE_OFFSET == PATCH_OFFSET (to save time)

                if (keyPressed(curBuffer, lastBuffer, PATCH_OFFSET, PATCH_UP)) {
                    curPatch = (curPatch + 1) % MIDI_NUMPATCHES;
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_PROGCH, curPatch, 0));
                }
                if (keyPressed(curBuffer, lastBuffer, PATCH_OFFSET, PATCH_DOWN)) {
                    curPatch -= 1;
                    if (curPatch < 0) {
                        curPatch = MIDI_NUMPATCHES - 1;
                    }
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_PROGCH, curPatch, 0));
                }

            }

            if (curBuffer[MOD_OFFSET] != lastBuffer[MOD_OFFSET] &&
                curBuffer[PBBTN_OFFSET] == lastBuffer[PBBTN_OFFSET]) {

                if ((curBuffer[PBBTN_OFFSET] & PBBTN_VALUE) == PBBTN_VALUE) {
                    // Pitch bend
                    uint8_t value = 0x40; // Reset value
                    if (curBuffer[MOD_OFFSET] != 0) {
                        value = curBuffer[MOD_OFFSET] - 1;
                        Pm_WriteShort(outStream, 0,
                            Pm_Message(MIDI_PBEND, 0, value));
                    }
                } else {
                    // Modulation
                    if (curBuffer[MOD_OFFSET] != 0) {
                        Pm_WriteShort(outStream, 0,
                            Pm_Message(MIDI_CTRLCH, 1, curBuffer[MOD_OFFSET] - 1));
                    }
                }
            }

            int curDpad = curBuffer[DPAD_OFFSET] & DPAD_MASK;
            int lastDpad = lastBuffer[DPAD_OFFSET] & DPAD_MASK;
            if (curDpad != DPAD_CENTER && curDpad != lastDpad) {
                switch (curDpad) {
                case DPAD_UP:
                    drumMapOn = !drumMapOn;
                    break;
                }
            }

            uint8_t t = 0;
            uint8_t keyIndex = 0;
            int velsKeyIndex = 0;
            uint8_t vel;
            uint8_t chan;
            uint8_t note;

            // Each byte in bitmap
            for (i = BITMAP_OFFSET; i < BITMAP_OFFSET + BITMAP_LEN; i++) {

                // Interferes with velocity array index calculations.
                //if (curBuffer[i] == lastBuffer[i]) {
                //    keyIndex += 8;
                //    continue;
                //}

                if (i == BITMAP_OFFSET + BITMAP_LEN - 1) {
                    // Last byte only has one bit to process.
                    t = 0x40;
                }

                // Each bit
                for (bitmask = 0x80; bitmask != t; bitmask >>= 1) {
                    cv = curBuffer[i] & bitmask;
                    lv = lastBuffer[i] & bitmask;
                    if (cv != lv) {
                        if (cv) {
                            // Note On
                            chan = NORMAL_CHAN;
                            note = firstNote + keyIndex;
                            //printf("\x90%c\x40", note);
                            //fprintf(stderr, "Sending NoteOn(%d)\n", note);
                            if (keyIndex < DRUMMAP_NKEYS) {
                                if (drumMapOn) {
                                    chan = DRUMMAP_CHAN;
                                    note = drumMapNotes[keyIndex];
                                }
                                chansDown[keyIndex] = chan;
                            }
                            vel = DEFAULT_VEL;
                            if (numKeysDown < VELS_LEN && velsKeyIndex < VELS_LEN) {
                                // N.B. accepting a few strange but harmless
                                // (and unlikely) edge cases here in the
                                // interests of efficiency.
                                vel = (0x7f & curBuffer[VELS_OFFSET + velsKeyIndex]);
                            }
                            Pm_WriteShort(outStream, 0,
                                Pm_Message(MIDI_NOTEON | (MIDI_CHANMASK & chan),
                                           note, vel));
                            notesDown[keyIndex] = note;
                            ++numKeysDown;
                        } else {
                            // Note Off
                            //printf("\x80%c\x40", notesDown[keyIndex]);
                            //fprintf(stderr, "Sending NoteOff(%d)\n", notesDown[keyIndex]);
                            chan = NORMAL_CHAN;
                            if (keyIndex < DRUMMAP_NKEYS) {
                                chan = chansDown[keyIndex];
                            }
                            Pm_WriteShort(outStream, 0,
                                Pm_Message(MIDI_NOTEOFF | (MIDI_CHANMASK & chan),
                                           notesDown[keyIndex], 0x40));
                            --numKeysDown;
                        }
                        //fflush(stdout);
                    }
                    if (cv != 0) {
                        ++velsKeyIndex;
                    }
                    ++keyIndex;
                }
            }

            // Swap buffers
            uint8_t *tmp = curBuffer;
            curBuffer = lastBuffer;
            lastBuffer = tmp;

        }

        transferred_len = 0;

    }

    return 0;
}
