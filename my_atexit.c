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
