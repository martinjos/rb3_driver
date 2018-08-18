LIBUSB_FLAGS = $(shell pkg-config libusb-1.0 --libs --cflags)
CCFLAGS += $(LIBUSB_FLAGS)
LDLIBS += $(LIBUSB_FLAGS) -lportmidi

all: rb3_driver

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CCFLAGS)

rb3_driver: rb3_driver.o myusb_utils.o myusb_atexit.o my_atexit.o
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) $^ $(LDLIBS)
