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
 * ownership and/or confer such rights of exclusion onto others.
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

void myusb_unref_device(void *data) {
    libusb_device *dev = (libusb_device *)data;
    libusb_unref_device(dev);
}

void myusb_exit(void *ignored) {
    libusb_exit(NULL);
}

void myusb_close(void *data) {
    libusb_device_handle *h = (libusb_device_handle *)data;
    libusb_close(h);
}

void myusb_free_config_descriptor(void *data) {
    struct libusb_config_descriptor *cfgDesc =
        (struct libusb_config_descriptor *)data;
    libusb_free_config_descriptor(cfgDesc);
}

typedef struct myusb_release_interface_data {
    libusb_device_handle *dev;
    int interface_number;
} myusb_release_interface_data;

static void myusb_release_interface(void *data) {
    myusb_release_interface_data *d = (myusb_release_interface_data *)data;
    libusb_release_interface(d->dev, d->interface_number);
    free(d);
}

void myusb_atexit_release_interface(libusb_device_handle *dev, int interface_number) {
    myusb_release_interface_data *d =
        (myusb_release_interface_data *)malloc(sizeof(myusb_release_interface_data));
    if (d == NULL) {
        return;
    }
    d->dev = dev;
    d->interface_number = interface_number;
    my_atexit(myusb_release_interface, d);
}
