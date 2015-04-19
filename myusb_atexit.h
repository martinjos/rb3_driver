#ifndef MYUSB_ATEXIT_H
#define MYUSB_ATEXIT_H 1

#include <libusb.h>

#include "my_atexit.h"

extern void myusb_unref_device(void *data);
extern void myusb_exit(void *ignored);
extern void myusb_close(void *data);
extern void myusb_free_config_descriptor(void *data);
extern void myusb_atexit_release_interface(libusb_device_handle *dev, int interface_number);

#endif /* MYUSB_ATEXIT_H */
