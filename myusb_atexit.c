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
