RB3 Wireless MIDI Keyboard Driver
=================================

This is a user-mode "driver" for the Harmonix Rock Band 3 wireless MIDI
keyboard ("keytar") in wireless mode.

So far, it has only been tested with the Wii version of the keytar, although I
see no reason in principle why most functionality should not work with the
other versions.


What is the keytar?
-------------------

The keytar is a 2-octave (25-key) keyboard with full-size spring-loaded
velocity-sensitive keys, octave shift and program change buttons, a
footswitch/modulation pedal jack, and a dual-function modulation/pitch-bend
touch panel.  It comes with a USB dongle that communicates wirelessly with the
keytar.  It also has a standard MIDI out (DIN) jack.


What is this driver supposed to do?
-----------------------------------

Using this driver (although I make no representation that it will work for
you), you can use the keytar to send MIDI messages wirelessly to your computer
using the dongle that comes with the device.

The dongle does not support standard USB-MIDI, so this driver converts the
dongle's USB packets into MIDI and sends the messages to a MIDI output
interface of your choosing.

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
normal user), giving the name of the MIDI output device as an argument.  If you
run the program with no arguments, it will scan your system and display a list
of MIDI output devices (to the console).

It should automatically detect the correct USB (input) device, provided that
the keytar dongle is plugged in.

Most systems will have at least one built-in MIDI output device that
corresponds to your sound card's or operating system's built-in MIDI
synthesizer.  If you install MIDI Yoke on Windows, it provides a set of 8
virtual MIDI output devices that send all their data to the corresponding
virtual MIDI input device (which has the same number but "In" instead of
"Out").


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
change buttons, the modulation/pitch-bend touch panel, and drum split are
almost fully implemented (I think the only thing missing is octave/program
reset).

Note that drum split requires a General MIDI-compliant synthesizer program.  If
your synthesizer program is not General MIDI compliant, it may output either
nothing at all, or a confusing array of jumbled up notes, when drum split is
active and the drum keys (i.e. the lower 12) are pressed.

All foot pedal features, the sequencer control buttons, and LED output are so
far not implemented.


License
-------

This software is licensed under the Apache License, Version 2.0 (the "License");
you may not use this software except in compliance with the License.
You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.


Additional legal disclaimers
----------------------------

The author of this software is not associated or affiliated with Harmonix Music
Systems, Inc., or Nintendo Co., Ltd. and this software is not in any way
endorsed or approved by those companies or their subsidiaries/associates.

The author takes no responsibility for any attempt to use this software with
Harmonix and/or Nintendo products, and provides no guarantee or affirmation
that it is legal to do so in your jurisdiction.

Harmonix and Rock Band are trademarks of Harmonix Music Systems, Inc.  Nintendo
and Wii are trademarks of Nintendo Co., Ltd.  The author does not claim any
rights over those trademarks.

