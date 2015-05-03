#!/usr/bin/env perl
#
# Copyright (c) 2015 Martin Sidaway
# 
# A perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
# copyright and patent license is hereby granted, free of charge, to any
# person obtaining a copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction, including
# without limitation (and in any combination) the rights to use, copy, modify,
# merge, reimplement, publish, distribute, sublicense, and/or sell copies of
# the Software (and/or portions thereof), and to permit persons to whom the
# Software is furnished to do so. This license includes a waiver of the
# authors', copyright-holders' and (where applicable) patent-holders' rights
# to exclude such activities on the basis of present or future patent
# ownership and/or confer such rights of exclusion onto others.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT OR PATENT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
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
