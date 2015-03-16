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

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "my_atexit.h"

typedef struct my_atexit_data {
    void (*func)(void *);
    void *data;
    struct my_atexit_data *next;
} my_atexit_data;

static my_atexit_data *my_atexit_handlers = NULL;

static void my_atexit_callback() {
    my_atexit_data *handler = NULL;
    while (my_atexit_handlers != NULL) {
        handler = my_atexit_handlers;
        my_atexit_handlers = my_atexit_handlers->next;
        handler->func(handler->data);
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
