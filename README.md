RB3 Wireless MIDI Keyboard Driver
=================================

This is a user-mode "driver" for the Harmonix Rock Band 3 wireless MIDI
keyboard ("keytar") in wireless mode.

So far, it has only been tested with the Wii version of the keytar, although I
see no reason in principle why most functionality should not work with the
other versions.

Please see the bottom of this document for licensing and disclaimers.


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
"Out" and "In" devices with corresponding numbers, and it should work.


Do I need to install it / is it complicated to use?
---------------------------------------------------

No, it's just a normal program.  The simplest way to use it (on Windows) is
just to expand the zip file to a folder, then double-click "rb3_driver" in the
folder.  You will be presented with a numbered list of MIDI output devices.
Just type the number of your chosen output device, and press the Enter key on
your keyboard.  If something goes wrong, the program will display an error
message, then exit automatically after a few seconds.  To close the program at
any time, just close the window.  While in use, you can minimise the program in
the normal way if you want to avoid cluttering up your screen.

It should automatically detect the correct USB (input) device, provided that
the keytar dongle is plugged in.

Most systems will have at least one built-in MIDI output device that
corresponds to your sound card's or operating system's built-in MIDI
synthesizer.  However, this will usually be of low quality, and you will
probably want to connect it to something else.  If you install MIDI Yoke on
Windows, it provides a set of 8 virtual MIDI output devices that send all their
data to the corresponding virtual MIDI input device (which has the same number
but "In" instead of "Out").  This works exactly like a digital patch cable.
You can set up your synthesizer or sequencer program to use this as its input.


What are the dependencies / how do I compile it?
------------------------------------------------

If you are using Windows, the good news is that you don't have to compile it
yourself.  A ready-compiled version of the program (with dependencies included)
is available from https://github.com/martinjos/rb3_driver/releases

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
change buttons, the modulation/pitch-bend touch panel, drum split, all foot
pedal features (including stomp and mode change), and the sequencer control
buttons, are fully implemented.  (Let me know if you find any bugs or missing
features.)

Note that drum split requires a General MIDI-compliant synthesizer program.  If
your synthesizer program is not General MIDI compliant, it may output either
nothing at all, or a confusing array of jumbled up notes, when drum split is
active and the drum keys (i.e. the lower 12) are pressed.

Also note that the velocity-sensitivity is limited to at most 5 simultaneous
keys - if 5 keys are held down and you strike another, it will register as
having a velocity of exactly 64 (50% of the maximum value), regardless of the
actual velocity.  This is an unavoidable limitation of the keytar's USB packet
format.

LED output is so far not implemented.


License
-------

All authors, contributors, and holders of copyright in this repository hereby
grant (to the extent to which they have the power to do so) all recipients of
the repository a perpetual, worldwide, non-exclusive, no-charge, royalty-free,
irrevocable copyright and patent (but not trademark) license to make absolutely
any use of the content of the repository, and the copyrights and patentable
inventions embodied in it, as though it (and the inventions) were in the public
domain. This includes but is not limited to the rights to use for any purpose,
copy/reproduce in any form, modify or produce derivative works, merge, publish,
distribute modified/derived or unmodified copies for any purpose with or
without fee, sublicense, sell, offer to sell, import or otherwise transfer,
publicly perform or display, make, have made, etc. If you modify your copy of
this repository, preserving this notice, and then redistribute the modified
version, or intentionally submit modifications to be merged into this
repository, then in so doing you assert that the notice automatically applies
to the modified repository/the modifications and all patentable inventions
embodied therein.

THIS REPOSITORY IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE REPOSITORY OR THE USE OR OTHER DEALINGS IN THE
REPOSITORY.


Additional legal disclaimers
----------------------------

The author can not be held responsible for the content, safety, security,
freeness from bugs or viruses, or fitness-for-purpose of websites
mentioned/linked to from this file/this repository, nor of the third-party
programs and libraries mentioned here or anywhere in this repository (even
those mentioned as dependencies).  If you download, install, or otherwise use
any of these programs and libraries, or visit any of the websites, you do so
entirely at your own risk.

The author of this software is not associated or affiliated with Harmonix Music
Systems, Inc., or Nintendo Co., Ltd. and this software is not in any way
endorsed or approved by those companies or their subsidiaries/associates.

The author takes no responsibility for any attempt to use this software with
Harmonix and/or Nintendo products, and provides no guarantee or affirmation
that it is legal to do so in your jurisdiction.

Harmonix and Rock Band are trademarks of Harmonix Music Systems, Inc.  Nintendo
and Wii are trademarks of Nintendo Co., Ltd.  The author does not claim any
rights over those trademarks.

