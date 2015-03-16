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

#ifndef MYUSB_UTILS_H
#define MYUSB_UTILS_H 1

#include <libusb.h>

extern libusb_device *myusb_get_device_by_prod_name_prefix(const char *prefix, int index);
extern const struct libusb_endpoint_descriptor *
myusb_get_endpoint(libusb_device *dev, uint8_t direction,
             uint8_t attrs_mask, uint8_t attrs, int index,
             uint8_t *interface_number);

#endif /* MYUSB_UTILS_H */
