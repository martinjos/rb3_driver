#include <libusb.h>
#include <string.h>
#include <stdio.h>

#define MAX_PRODUCT_LEN 1024

libusb_device *get_device_by_prod_name_prefix(libusb_device **devs, ssize_t cnt,
                                              const char *prefix) {
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
            result = devs[i];
        }
        libusb_close(h);
    }
    return result;
}

int main(int argc, char **argv) {
    int r;

    r = libusb_init(NULL);
    if (r < 0) {
        return r;
    }

    libusb_device **devs;
    ssize_t cnt = libusb_get_device_list(NULL, &devs);
    if (cnt < 0) {
        fprintf(stderr, "List is empty\n");
        return (int)cnt;
    }

    libusb_device *dev =
        get_device_by_prod_name_prefix(devs, cnt, "Harmonix RB3 Keyboard");

    if (dev == NULL) {
        fprintf(stderr, "Failed to find device\n");
    } else {
        fprintf(stderr, "Brilliant news! Found device!\n");
    }

    libusb_free_device_list(devs, 1);
    libusb_exit(NULL);

    return 0;
}
