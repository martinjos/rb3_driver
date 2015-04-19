#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>

#ifdef _POSIX_SOURCE
    #include <unistd.h>
#endif

#ifdef _WIN32
    #include <windows.h>
    #define sleep(x) Sleep(1000*(x))
#endif

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
#define DEFAULT_1STNOTE  48
#define DEFAULT_PATCH    0
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
#define PED_OFFSET       14
#define MOD_OFFSET       15
#define PEDST_OFFSET     20
#define PEDST_REST       0
#define PEDST_STOMP      1
#define PEDST_PARTIAL    2
#define PEDST_FULL       3
#define SEQ_OFFSET       1
#define SEQ_STOP         0x01
#define SEQ_CONT         0x10
#define SEQ_START        0x02

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
#define MIDI_SEQSTART    0xFA
#define MIDI_SEQCONT     0xFB
#define MIDI_SEQSTOP     0xFC

#define MIDI_CC_MOD      1
#define MIDI_CC_FOOT     4
#define MIDI_CC_VOL      7
#define MIDI_CC_EXP      11
#define MIDI_CC_DAMP     64
#define MIDI_CM_ALLOFF   0x7B

#define MIDI_CHANMASK    0xf
#define MIDI_NUMPATCHES  0x80

#define SLEEP_IF_CHOOSEDEVICE() do { if (chooseDevice) sleep(3); } while (0)

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

static inline int keyDown(const char *buffer, int index, int mask) {
    return (buffer[index] & mask) == mask;
}

static inline int keyPressed(const char *curBuffer, const char *lastBuffer,
                             int index, int mask) {
    return (curBuffer[index] & mask) == mask && (lastBuffer[index] & mask) == 0;
}

int main(int argc, char **argv) {
    int r;
    int i, j, k;
    int chooseDevice = 0;

    if (argc < 2) {
        fprintf(stderr, "\nAvailable output devices:\n\n");
        chooseDevice = 1;
        //return 4;
    }

    /* initialize portmidi */
    r = Pm_Initialize();
    if (r < 0) {
        fprintf(stderr, "Failed to initialize portmidi\n");
        SLEEP_IF_CHOOSEDEVICE();
        return r;
    };
    my_atexit(mypm_terminate, NULL);

    int pmdCount = Pm_CountDevices();
    //fprintf(stderr, "Got %d portmidi devices\n", pmdCount);
    PmDeviceID pmDev = -1;
    for (i = 0, j = 1; i < pmdCount; ++i) {
        const PmDeviceInfo *pmdInfo = Pm_GetDeviceInfo(i);
        if (pmdInfo != NULL && pmdInfo->output != 0) {
            if (chooseDevice) {
                fprintf(stderr, " %d: \"%s\"\n", j, pmdInfo->name);
            } else if (strcmp(argv[1], pmdInfo->name) == 0) {
                pmDev = i;
            }
            ++j;
        }
    }
    if (chooseDevice) {
        fprintf(stderr, "\nPlease type the number of your chosen output MIDI device and press the Enter key.\n\n");
        scanf("%d", &k);
        fprintf(stderr, "\n");
        for (i = 0, j = 1; i < pmdCount; ++i) {
            const PmDeviceInfo *pmdInfo = Pm_GetDeviceInfo(i);
            if (pmdInfo != NULL && pmdInfo->output != 0) {
                if (j == k) {
                    pmDev = i;
                    break;
                }
                ++j;
            }
        }
    }
    if (pmDev == -1) {
        fprintf(stderr, "Unable to find MIDI output device\n");
        SLEEP_IF_CHOOSEDEVICE();
        return 3;
    }

    PortMidiStream *outStream;
    r = Pm_OpenOutput(&outStream, pmDev, NULL, 0, NULL, NULL, 0);
    if (r < 0) {
        fprintf(stderr, "Failed to open MIDI output device\n");
        SLEEP_IF_CHOOSEDEVICE();
        return r;
    }
    my_atexit(mypm_close, outStream);

    fprintf(stderr, "Got MIDI output device!\n");

    /* Initialize libusb */
    r = libusb_init(NULL);
    if (r < 0) {
        fprintf(stderr, "Failed to initialize libusb\n");
        SLEEP_IF_CHOOSEDEVICE();
        return r;
    }
    my_atexit(myusb_exit, NULL);

    libusb_device *dev =
        myusb_get_device_by_prod_name_prefix("Harmonix RB3 Keyboard", 0);

    if (dev == NULL) {
        fprintf(stderr, "Failed to find input device\n");
        SLEEP_IF_CHOOSEDEVICE();
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
        SLEEP_IF_CHOOSEDEVICE();
        return 2;
    }

    fprintf(stderr, "Got input device endpoint!\n");

    libusb_device_handle *h = NULL;
    r = libusb_open(dev, &h);
    if (r < 0) {
        fprintf(stderr, "Failed to open input device\n");
        SLEEP_IF_CHOOSEDEVICE();
        return r;
    }
    my_atexit(myusb_close, h);

    r = libusb_claim_interface(h, interface_number);
    if (r < 0) {
        fprintf(stderr, "Failed to claim input device interface\n");
        SLEEP_IF_CHOOSEDEVICE();
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

    // MIDI state
    int numKeysDown = 0;
    uint8_t notesDown[NUM_KEYS]; // What note was last activated for a given key?
    uint8_t chansDown[DRUMMAP_NKEYS]; // What channel was last activated for a given key?
    uint8_t firstNote = DEFAULT_1STNOTE;
    int8_t curPatch = DEFAULT_PATCH;
    int8_t drumMapOn = 0;
    uint8_t pedalCC = MIDI_CC_EXP;
    
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

            // Octave and program change

            if (curBuffer[OCTAVE_OFFSET] != lastBuffer[OCTAVE_OFFSET]) {

                uint8_t upOctPressed = keyPressed(curBuffer, lastBuffer, OCTAVE_OFFSET, OCTAVE_UP);
                uint8_t downOctPressed = keyPressed(curBuffer, lastBuffer, OCTAVE_OFFSET, OCTAVE_DOWN);

                if ((upOctPressed || downOctPressed) &&
                    keyDown(curBuffer, OCTAVE_OFFSET, OCTAVE_UP) &&
                    keyDown(curBuffer, OCTAVE_OFFSET, OCTAVE_DOWN)) {

                    firstNote = DEFAULT_1STNOTE;

                } else if (upOctPressed && firstNote <= 84) {
                    firstNote += 12;
                } else if (downOctPressed && firstNote >= 12) {
                    firstNote -= 12;
                }

                // N.B. assuming OCTAVE_OFFSET == PATCH_OFFSET (to save time)

                uint8_t upPatchPressed = keyPressed(curBuffer, lastBuffer, PATCH_OFFSET, PATCH_UP);
                uint8_t downPatchPressed = keyPressed(curBuffer, lastBuffer, PATCH_OFFSET, PATCH_DOWN);

                if ((upPatchPressed || downPatchPressed) &&
                    keyDown(curBuffer, PATCH_OFFSET, PATCH_UP) &&
                    keyDown(curBuffer, PATCH_OFFSET, PATCH_DOWN)) {

                    curPatch = DEFAULT_PATCH;
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_PROGCH, curPatch, 0));

                } else if (upPatchPressed) {
                    curPatch = (curPatch + 1) % MIDI_NUMPATCHES;
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_PROGCH, curPatch, 0));

                } else if (downPatchPressed) {
                    curPatch -= 1;
                    if (curPatch < 0) {
                        curPatch = MIDI_NUMPATCHES - 1;
                    }
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_PROGCH, curPatch, 0));
                }

            }

            // Sequencer controls

            if (curBuffer[SEQ_OFFSET] != lastBuffer[SEQ_OFFSET]) {
                uint8_t start = keyPressed(curBuffer, lastBuffer, SEQ_OFFSET, SEQ_START);
                uint8_t cont = keyPressed(curBuffer, lastBuffer, SEQ_OFFSET, SEQ_CONT);
                uint8_t stop = keyPressed(curBuffer, lastBuffer, SEQ_OFFSET, SEQ_STOP);

                if ((start || cont || stop) &&
                    keyDown(curBuffer, SEQ_OFFSET, SEQ_START) &&
                    keyDown(curBuffer, SEQ_OFFSET, SEQ_CONT) &&
                    keyDown(curBuffer, SEQ_OFFSET, SEQ_STOP)) {

                    // Turn off all notes on all channels.

                    // First, cancel any sequence commands that may have been
                    // started by mistake.
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_SEQSTOP, 0, 0));

                    Pm_WriteShort(outStream, 0,
                        Pm_Message(MIDI_CTRLCH | NORMAL_CHAN, MIDI_CM_ALLOFF, 0));
                    Pm_WriteShort(outStream, 0,
                        Pm_Message(MIDI_CTRLCH | DRUMMAP_CHAN, MIDI_CM_ALLOFF, 0));

                } else if (start) {
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_SEQSTART, 0, 0));
                } else if (cont) {
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_SEQCONT, 0, 0));
                } else if (stop) {
                    Pm_WriteShort(outStream, 0, Pm_Message(MIDI_SEQSTOP, 0, 0));
                }
            }

            // Modulation and pitch bend

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
                            Pm_Message(MIDI_CTRLCH, MIDI_CC_MOD,
                                       curBuffer[MOD_OFFSET] - 1));
                    }
                }
            }

            // Drum split & pedal mode

            int curDpad = curBuffer[DPAD_OFFSET] & DPAD_MASK;
            int lastDpad = lastBuffer[DPAD_OFFSET] & DPAD_MASK;
            if (curDpad != DPAD_CENTER && curDpad != lastDpad) {
                uint8_t newPedalCC = pedalCC;
                switch (curDpad) {
                case DPAD_UP:
                    drumMapOn = !drumMapOn;
                    break;
                case DPAD_LEFT:
                    newPedalCC = MIDI_CC_EXP;
                    break;
                case DPAD_DOWN:
                    newPedalCC = MIDI_CC_VOL;
                    break;
                case DPAD_RIGHT:
                    newPedalCC = MIDI_CC_FOOT;
                    break;
                }
                if (newPedalCC != pedalCC) {
                    // Reset old pedal to rest state
                    // Don't do this - the keyboard doesn't.
                    // It may be useful to "hold" a value of one pedal while
                    // changing another.
                    //Pm_WriteShort(outStream, 0,
                    //    Pm_Message(MIDI_CTRLCH, pedalCC, 0x7F));
                    pedalCC = newPedalCC;
                }
            }

            // Pedal value

            uint8_t curPedal = curBuffer[PED_OFFSET];
            uint8_t lastPedal = lastBuffer[PED_OFFSET];
            if (curPedal != lastPedal) {
                uint8_t curAnalog = curPedal & 0x7F;
                uint8_t lastAnalog = lastPedal & 0x7F;
                if (curAnalog != lastAnalog) {
                    Pm_WriteShort(outStream, 0,
                        Pm_Message(MIDI_CTRLCH, pedalCC, curAnalog));
                }
                uint8_t curDigital = curPedal & 0x80;
                uint8_t lastDigital = lastPedal & 0x80;
                if (curDigital != lastDigital) {
                    Pm_WriteShort(outStream, 0,
                        Pm_Message(MIDI_CTRLCH, MIDI_CC_DAMP,
                                   curDigital ? 0x7F : 0));
                }
            }

            // Keys

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
