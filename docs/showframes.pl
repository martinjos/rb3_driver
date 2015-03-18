#!/usr/bin/env perl
#
#  Copyright 2015 Martin Sidaway
#
#  Licensed under the Apache License, Version 2.0 (the "License");
#  you may not use this file except in compliance with the License.
#  You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#

# This script is provided for reference purposes.  It reads successive 27-byte
# frames from a device file and displays the information in a user-readable
# format.  It has only been tested on OpenBSD.  It takes the device filename as
# an optional argument.

use warnings;
use strict;

my $continue = 1;
$SIG{INT} = sub {
    $continue = 0;
};

my $filename = scalar(@ARGV) > 0 ? $ARGV[0] : '/dev/uhid0';

open(my $fh, '<', $filename);
my $lastframe = "";

while ($continue) {
    read($fh, my $frame, 27, 0);
    next if $frame eq $lastframe;
    $lastframe = $frame;

    my ($btns, $dpad, $keystr, $keys, $velstr, $vels, $pbb, $on, $mod, $seq);
    ($btns, $dpad, $_, $keystr, $velstr, $pbb, $on, $mod, $_, $seq, $_) =
        unpack('nCa2a3a5CCCa9CC', $frame);

    $keys = unpack('N', substr($frame, 5, 4));
    $keys = $keys >> 7;

    if (length($velstr) > 0) {
        vec($velstr, 0, 8) = vec($velstr, 0, 8) & 0x7f;
    }
    $vels = [];
    @{$vels} = unpack('C*', $velstr);

    printf("btns: %04x, dpad: %d, keys: %07x, vels: [%s], pbb: %02x, on: %02x, mod: %02x, seq: %02x\n", $btns, $dpad, $keys, join(', ', map(sprintf('%02x', $_), @{$vels})), $pbb, $on, $mod, $seq);
}

close($fh);
