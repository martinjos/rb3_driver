/*
 * Copyright (c) 2015 Martin Sidaway
 *
 * A perpetual, worldwide, non-exclusive, no-charge, royalty-free, irrevocable
 * copyright and patent license is hereby granted, free of charge, to any
 * person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including
 * without limitation (and in any combination) the rights to use, copy, modify,
 * merge, reimplement, publish, distribute, sublicense, and/or sell copies of
 * the Software (and/or portions thereof), and to permit persons to whom the
 * Software is furnished to do so. This license includes a waiver of the
 * authors', copyright-holders' and (where applicable) patent-holders' rights
 * to exclude such activities on the basis of present or future patent
 * ownership and/or confer such rights of exclusion onto others, insofar as
 * such patents pertain to the patent-holders' contributions to the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT OR PATENT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef MY_ATEXIT_H
#define MY_ATEXIT_H 1

extern void my_atexit(void (*func)(void *), void *data);

#endif /* MY_ATEXIT_H */
