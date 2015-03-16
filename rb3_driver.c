#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "myusb_atexit.h"
#include "myusb_utils.h"

#define DATA_BUFFER_LEN  27
#define TRANSFER_TIMEOUT 500

int main(int argc, char **argv) {
    int r;

    r = libusb_init(NULL);
    if (r < 0) {
        return r;
    }
    my_atexit(myusb_exit, NULL);

    libusb_device *dev =
        myusb_get_device_by_prod_name_prefix("Harmonix RB3 Keyboard", 0);

    if (dev == NULL) {
        fprintf(stderr, "Failed to find device\n");
        return 1;
    }

    fprintf(stderr, "Brilliant news! Found device!\n");

    uint8_t interface_number = 0;
    const struct libusb_endpoint_descriptor *endpoint =
        myusb_get_endpoint(dev, LIBUSB_ENDPOINT_IN,
                     LIBUSB_TRANSFER_TYPE_MASK, LIBUSB_TRANSFER_TYPE_INTERRUPT, 0,
                     &interface_number);

    if (endpoint == NULL) {
        fprintf(stderr, "No suitable endpoint\n");
        return 2;
    }

    fprintf(stderr, "Got endpoint!\n");

    libusb_device_handle *h = NULL;
    r = libusb_open(dev, &h);
    if (r < 0) {
        fprintf(stderr, "Failed to open device\n");
        return r;
    }
    my_atexit(myusb_close, h);

    r = libusb_claim_interface(h, interface_number);
    if (r < 0) {
        fprintf(stderr, "Failed to claim interface\n");
        return r;
    }
    myusb_atexit_release_interface(h, interface_number);

    uint8_t buffer1[DATA_BUFFER_LEN];
    uint8_t buffer2[DATA_BUFFER_LEN];
    memset(buffer1, 0, sizeof(buffer1));
    memset(buffer2, 0, sizeof(buffer2));
    uint8_t *curBuffer = buffer1;
    uint8_t *otherBuffer = buffer2;
    int transferred_len = 0;
    
    //struct libusb_transfer transfer;
    //libusb_fill_interrupt_transfer(&transfer, h, endpoint->bEndpointAddress, buffer, DATA_BUFFER_LEN, got_data, NULL, TRANSFER_TIMEOUT);

    int i;
    while (1) {

        r = libusb_interrupt_transfer(h, endpoint->bEndpointAddress, curBuffer, DATA_BUFFER_LEN, &transferred_len, TRANSFER_TIMEOUT);

        if (r == LIBUSB_ERROR_TIMEOUT) {
            fprintf(stderr, "Data transfer timed out\n");
            continue;
        }

        if (r < 0 || transferred_len == 0) {
            // N.B. this happens when the USB dongle is removed.
            fprintf(stderr, "Data transfer failed\n");
            break;
        }

        if (transferred_len < DATA_BUFFER_LEN) {
            fprintf(stderr, "Wrong packet size\n");
            continue;
        }

        if (memcmp(curBuffer, otherBuffer, DATA_BUFFER_LEN) != 0) {

            for (i = 0; i < DATA_BUFFER_LEN; i++) {
                fprintf(stderr, " %02x", curBuffer[i]);
            }
            fprintf(stderr, "\n");

            // Swap buffers
            uint8_t *tmp = curBuffer;
            curBuffer = otherBuffer;
            otherBuffer = tmp;

        }

        transferred_len = 0;

    }

    return 0;
}
