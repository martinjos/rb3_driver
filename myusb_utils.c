/*
 * Copyright (c) 2015 Martin Sidaway
 *
 * A perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
 * copyright and patent license is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation (and in any combination) the rights to use, copy, modify,
 * merge, reimplement, publish, distribute, sublicense, and/or sell copies of
 * the Software (and/or portions thereof), and to permit persons to whom the
 * Software is furnished to do so. This license includes a waiver of the
 * authors', copyright-holders' and (where applicable) patent-holders' rights
 * to exclude such activities on the basis of present or future patent
 * ownership and/or confer such rights of exclusion onto others, insofar as
 * such patents pertain to the patent-holders' contributions to the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT OR PATENT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "myusb_atexit.h"
#include "myusb_utils.h"

#define MAX_PRODUCT_LEN 1024

libusb_device *myusb_get_device_by_prod_name_prefix(const char *prefix, int index) {

    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        fprintf(stderr, "Failed to get device list\n");
        return NULL;
    }

    ssize_t i;
    struct libusb_device_descriptor desc;
    libusb_device_handle *h;
    libusb_device *result = NULL;
    char prodName[MAX_PRODUCT_LEN];
    int r;
    size_t prefixLen = strlen(prefix);
    for (i = 0; i < cnt && !result; i++) {
        r = libusb_get_device_descriptor(devs[i], &desc);
        if (r < 0) {
            continue;
        }
        r = libusb_open(devs[i], &h);
        if (r < 0) {
            continue;
        }
        r = libusb_get_string_descriptor_ascii(h, desc.iProduct,
                                               prodName, MAX_PRODUCT_LEN);
        if (r >= 0 && strncmp(prefix, prodName, prefixLen) == 0) {
            if (index == 0) {
                result = devs[i];
            } else {
                --index;
            }
        }
        libusb_close(h);
    }

    if (result != NULL) {
        // Increase reference count of device before freeing list.
        libusb_ref_device(result);
        my_atexit(myusb_unref_device, result);
    }

    libusb_free_device_list(devs, 1);

    return result;
}

const struct libusb_endpoint_descriptor *
myusb_get_endpoint(libusb_device *dev, uint8_t direction,
             uint8_t attrs_mask, uint8_t attrs, int index,
             uint8_t *interface_number) {

    struct libusb_config_descriptor *cfgDesc;
    int r = libusb_get_active_config_descriptor(dev, &cfgDesc);
    if (r < 0) {
        fprintf(stderr, "Failed to get active config\n");
        return NULL;
    }
    my_atexit(myusb_free_config_descriptor, cfgDesc);

    const struct libusb_endpoint_descriptor *endpoint = NULL;
    int i, j, k;
    for (i = 0; i < cfgDesc->bNumInterfaces; i++) {
        for (j = 0; j < cfgDesc->interface[i].num_altsetting; j++) {

            uint8_t numEndpoints = cfgDesc->interface[i].altsetting[j].bNumEndpoints;
            const struct libusb_endpoint_descriptor *endpoints = &cfgDesc->interface[i].altsetting[j].endpoint[0];

            for (k = 0; k < numEndpoints; k++) {
                if ((endpoints[k].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == direction &&
                    (endpoints[k].bmAttributes & attrs_mask) == attrs) {
                    if (index == 0) {
                        endpoint = &endpoints[k];
                        *interface_number = cfgDesc->interface[i].altsetting[j].bInterfaceNumber;
                        goto break_outer;
                    } else {
                        --index;
                    }
                }
            }
        }
    }
break_outer:

    return endpoint;
}
