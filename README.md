RB3 Wireless MIDI Keyboard Driver
=================================

This is a user-mode "driver" for the Harmonix (TM) Rock Band (TM) 3 wireless
MIDI keyboard ("keytar") in wireless mode.

So far, it has only been tested with the Wii version of the keytar, although I
see no reason in principle why most functionality should not work with the
other versions.


What is the keytar?
-------------------

The keytar is a 2-octave (25-key) keyboard with full-size spring-loaded keys,
octave shift and program change buttons, a footswitch/modulation pedal jack,
and a dual-function modulation/pitch-bend touch panel.  It comes with a USB
dongle that communicates wirelessly with the keytar.  It also has a standard
MIDI out (DIN) jack.


What is this driver supposed to do?
-----------------------------------

Using this driver (although I make no representation that it will work for
you), you can use the keytar to send MIDI messages wirelessly to your computer
using the dongle that comes with the device.

The dongle does not support standard USB-MIDI, so this driver converts the
dongle's USB packets into MIDI and sends the messages to a MIDI output
interface of your choosing.  If you run the program (in a console) with no
arguments, it will scan your system and display a list of MIDI output devices.

To use the driver's output as input to another program, you need some way of
patching the MIDI "Out" of this program to the MIDI "In" of the other program.
On Windows, this can be done using MIDI Yoke (http://www.midiox.com).  On Linux
or Mac OS X, you can probably use Jack (and I believe Jack 2 supports Windows
as well).  However, so far I haven't tried it with anything apart from MIDI
Yoke.

If you do use MIDI Yoke (on Windows), bear in mind that you do *not* need to
manually create a patch using MIDI-OX.  In fact, if you do this, it will not
work properly.  [I found this out the hard way. :-)]  Simply use the MIDI Yoke
"Out" and "In" devices with corresponding numbers, and it will work.


Do I need to install it / is it complicated to use?
---------------------------------------------------

No, it's just a normal program.  You just compile the program, and run it (as a
normal user), giving the name of the MIDI output device as an argument.  It
should automatically detect the correct USB device, provided that the keytar
dongle is plugged in.


What are the dependencies / how do I compile it?
------------------------------------------------

So far, I have only compiled it under MinGW32/MSYS.  However, it should in
principle work just as well on Linux or even (possibly) Mac OS X.

The main dependencies are libusb (http://libusb.org/) and PortMidi
(http://portmedia.sourceforge.net/portmidi/).

Once you have those in place (and your C compiler/build system obviously!),
just type "make".  (I think this may require GNU make - if it doesn't work, it
should be simple enough to adapt the Makefile.)


How much of the keytar's functionality is implemented?
------------------------------------------------------

At the moment, the keys themselves (including velocity), octave & program
change buttons, and modulation/pitch-bend touch panel are almost fully
implemented (I think the only thing missing is octave/program reset).

Drum split, all foot pedal features, and the sequencer control buttons are so
far not implemented.  The output LEDs are also not supported at present.


Legal disclaimers
-----------------

The author of this software is not associated or affiliated with Harmonix Music
Systems, Inc., and this software is not in any way endorsed or approved by
that company or its associates.

This software is provided "as-is", for educational purposes only.  The author
takes no responsibility for any attempt to use this software with Harmonix
products, and provides no guarantee or affirmation that it is legal to do so in
your jurisdiction.

A full disclaimer of warranty may be found in the Apache license, version 2.0,
under which this software is licensed, available from
http://www.apache.org/licenses/LICENSE-2.0

Harmonix and Rock Band are trademarks of Harmonix Music Systems, Inc.

