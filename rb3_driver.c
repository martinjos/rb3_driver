#include <libusb.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_PRODUCT_LEN  1024
#define DATA_BUFFER_LEN  27
#define TRANSFER_TIMEOUT 500

typedef struct my_atexit_data {
    void (*func)(void *);
    void *data;
    struct my_atexit_data *next;
} my_atexit_data;

static my_atexit_data *my_atexit_handlers = NULL;

void my_atexit_callback() {
    my_atexit_data *handler = NULL;
    while (my_atexit_handlers != NULL) {
        handler = my_atexit_handlers;
        my_atexit_handlers = my_atexit_handlers->next;
        handler->func(handler->next);
        free(handler);
    }
}

static int my_atexit_initialized = 0;
void my_atexit(void (*func)(void *), void *data) {
    if (my_atexit_initialized == 0) {
        my_atexit_initialized = 1;
        atexit(my_atexit_callback);
    }
    my_atexit_data *new_handler =
        (my_atexit_data *) malloc(sizeof(my_atexit_data));
    if (new_handler == NULL) {
        return;
    }
    new_handler->func = func;
    new_handler->data = data;
    new_handler->next = my_atexit_handlers;
    my_atexit_handlers = new_handler;
}

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
void myusb_release_interface(void *data) {
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

libusb_device *get_device_by_prod_name_prefix(const char *prefix, int index) {

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

int main(int argc, char **argv) {
    int r;

    r = libusb_init(NULL);
    if (r < 0) {
        return r;
    }
    my_atexit(myusb_exit, NULL);

    libusb_device *dev =
        get_device_by_prod_name_prefix("Harmonix RB3 Keyboard", 0);

    if (dev == NULL) {
        fprintf(stderr, "Failed to find device\n");
        return 1;
    }

    fprintf(stderr, "Brilliant news! Found device!\n");

    struct libusb_config_descriptor *cfgDesc;
    r = libusb_get_active_config_descriptor(dev, &cfgDesc);
    if (r < 0) {
        fprintf(stderr, "Failed to get active config\n");
        return r;
    }
    my_atexit(myusb_free_config_descriptor, cfgDesc);

    if (cfgDesc->bNumInterfaces < 1 || cfgDesc->interface[0].num_altsetting < 1 || cfgDesc->interface[0].altsetting[0].bNumEndpoints < 1) {
        fprintf(stderr, "No endpoints found\n");
        return 2;
    }

    uint8_t numEndpoints = cfgDesc->interface[0].altsetting[0].bNumEndpoints;
    const struct libusb_endpoint_descriptor *endpoints = &cfgDesc->interface[0].altsetting[0].endpoint[0];
    const struct libusb_endpoint_descriptor *endpoint = NULL;

    int i;
    for (i = 0; i < numEndpoints; i++) {
        if ((endpoints[i].bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_IN &&
            (endpoints[i].bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_INTERRUPT) {
            endpoint = &endpoints[i];
        }
    }

    if (endpoint == NULL) {
        fprintf(stderr, "No suitable endpoint\n");
        return 3;
    }

    fprintf(stderr, "Got endpoint!\n");

    libusb_device_handle *h = NULL;
    r = libusb_open(dev, &h);
    if (r < 0) {
        fprintf(stderr, "Failed to open device\n");
        return r;
    }
    my_atexit(myusb_close, h);

    r = libusb_claim_interface(h, 0);
    if (r < 0) {
        fprintf(stderr, "Failed to claim interface\n");
        return r;
    }
    myusb_atexit_release_interface(h, 0);

    uint8_t buffer1[DATA_BUFFER_LEN];
    uint8_t buffer2[DATA_BUFFER_LEN];
    memset(buffer1, 0, sizeof(buffer1));
    memset(buffer2, 0, sizeof(buffer2));
    uint8_t *curBuffer = buffer1;
    uint8_t *otherBuffer = buffer2;
    int transferred_len = 0;
    
    //struct libusb_transfer transfer;
    //libusb_fill_interrupt_transfer(&transfer, h, endpoint->bEndpointAddress, buffer, DATA_BUFFER_LEN, got_data, NULL, TRANSFER_TIMEOUT);

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
