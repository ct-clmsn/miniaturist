//  Copyright (c) 2021 Christopher Taylor
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
//  code below is slightly modified from MIT-licensed code from musl
/*
musl libc

musl, pronounced like the word "mussel", is an MIT-licensed
implementation of the standard C library targetting the Linux syscall
API, suitable for use in a wide range of deployment environments. musl
offers efficient static and dynamic linking support, lightweight code
and low runtime overhead, strong fail-safe guarantees under correct
usage, and correctness in the sense of standards conformance and
safety. musl is built on the principle that these goals are best
achieved through simple code that is easy to understand and maintain.

The 1.1 release series for musl features coverage for all interfaces
defined in ISO C99 and POSIX 2008 base, along with a number of
non-standardized interfaces for compatibility with Linux, BSD, and
glibc functionality.

For basic installation instructions, see the included INSTALL file.
Information on full musl-targeted compiler toolchains, system
bootstrapping, and Linux distributions built on musl can be found on
the project website:

    http://www.musl-libc.org/
*/
#pragma once
#ifndef __MINIATURIST_DRAND_HPP__
#define __MINIATURIST_DRAND_HPP__

#include <cstdint>

struct drand {
    unsigned short __seed48[7];

    drand() : __seed48{ 0, 0, 0, 0xe66d, 0xdeec, 0x5, 0xb } {
    }

    uint64_t __rand48_step(unsigned short *xi, unsigned short *lc) {
        uint64_t a, x;
        x = xi[0] | ((xi[1]+0U)<<16) | ((xi[2]+0ULL)<<32);
        a = lc[0] | ((lc[1]+0U)<<16) | ((lc[2]+0ULL)<<32);
        x = a*x + lc[3];
        xi[0] = x;
        xi[1] = x>>16;
        xi[2] = x>>32;
        return x & 0xffffffffffffull;
    }

    double erand48(unsigned short s[3]) {
	union {
		uint64_t u;
		double f;
	} x = { 0x3ff0000000000000ULL | __rand48_step(s, __seed48+3)<<4 };
	return x.f - 1.0;
    }

    double operator()() {
        return erand48(__seed48);
    }
};

#endif
