#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <portmidi.h>

#include "myusb_atexit.h"
#include "myusb_utils.h"

#define DATA_BUFFER_LEN  27
#define TRANSFER_TIMEOUT 500

void mypm_terminate(void *ignored) {
    Pm_Terminate();
}

void mypm_close(void *data) {
    PortMidiStream *str = (PortMidiStream *)data;
    Pm_Close(str);
}

int main(int argc, char **argv) {
    int r;
    int i;
    int printUsage = 0;

    if (argc < 2) {
        fprintf(stderr, "\nUsage: rb3_driver 'MIDI output device name'\n");
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
                fprintf(stderr, "  '%s'\n", pmdInfo->name);
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
    
    //struct libusb_transfer transfer;
    //libusb_fill_interrupt_transfer(&transfer, h, endpoint->bEndpointAddress, buffer, DATA_BUFFER_LEN, got_data, NULL, TRANSFER_TIMEOUT);

    uint8_t bitmask, t, note;
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

            //for (i = 0; i < DATA_BUFFER_LEN; i++) {
            //    fprintf(stderr, " %02x", curBuffer[i]);
            //}
            //fprintf(stderr, "\n");

            // Each byte in bitmap
            t = 0;
            note = 48;
            for (i = 5; i < 9; i++) {
                if (curBuffer[i] == lastBuffer[i]) {
                    note += 8;
                    continue;
                }
                if (i == 8) {
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
                            //printf("\x90%c\x40", note);
                            //fprintf(stderr, "Sending NoteOn(%d)\n", note);
                            Pm_WriteShort(outStream, 0, Pm_Message(0x90, note, 0x40));
                        } else {
                            // Note Off
                            //printf("\x80%c\x40", note);
                            //fprintf(stderr, "Sending NoteOff(%d)\n", note);
                            Pm_WriteShort(outStream, 0, Pm_Message(0x80, note, 0x40));
                        }
                        //fflush(stdout);
                    }
                    ++note;
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
