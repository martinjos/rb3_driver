#ifndef MYUSB_UTILS_H
#define MYUSB_UTILS_H 1

#include <libusb.h>

extern libusb_device *myusb_get_device_by_prod_name_prefix(const char *prefix, int index);
extern const struct libusb_endpoint_descriptor *
myusb_get_endpoint(libusb_device *dev, uint8_t direction,
             uint8_t attrs_mask, uint8_t attrs, int index,
             uint8_t *interface_number);

#endif /* MYUSB_UTILS_H */
