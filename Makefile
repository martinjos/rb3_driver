LDLIBS += -lusb-1.0

all: rb3_driver

rb3_driver: rb3_driver.o myusb_utils.o myusb_atexit.o my_atexit.o
	$(CC) -o $@ $(CCFLAGS) $(LDFLAGS) $^ $(LDLIBS)
