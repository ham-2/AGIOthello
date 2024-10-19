#include "misc.h"

PRNG rng(3245356235923498ULL);

/* The MIT License

   Copyright (C) 2012 Zilong Tan (eric.zltan@gmail.com)

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

inline uint64_t mix(uint64_t i) {
	i ^= i >> 23;
	i *= 0x2127599bf4325c37ULL;
	return i ^ (i >> 47);
}

uint64_t hash_128i(uint64_t upper, uint64_t lower)
{
	constexpr uint64_t    m = 0x880355f21e6d1965ULL;
	constexpr uint64_t seed = 9235123129483259312ULL;

	uint64_t h = seed;
	h ^= mix(upper);
	//h *= m;
	h ^= mix(lower);
	//h *= m;

	return mix(h);
}