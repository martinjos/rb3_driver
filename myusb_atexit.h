/*
 *  Copyright 2015 Martin Sidaway
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

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
