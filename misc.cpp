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

int load_file(char* dst, std::string filename, size_t size)
{
	std::cout << "Loading \"" << filename << "\"\n";
	std::ifstream input(filename, std::ios::binary);

	input.read(dst, size);

	if (input.fail() || (input.peek() != EOF)) {
		std::cout << "Failed to load" << std::endl;
		return -1;
	}
	else {
		std::cout << "Loaded \"" << filename << "\"" << std::endl;
	}

	input.close();
	return 0;
}

void save_file(char* src, std::string filename, size_t size)
{
	std::cout << "Saving to \"" << filename << "\"\n";
	std::ofstream output(filename, std::ios::binary);

	output.write(src, size);

	if (output.fail()) {
		std::cout << "Failed to save" << std::endl;
	}
	else {
		std::cout << "Saved to \"" << filename << "\"" << std::endl;
	}

	output.close();
}

void encode_literal(char* src, size_t size) {
	size_t s = 0;

	std::cout << "\n{ R\"(";
	for (; s < size; s += 6) {
		if (s % (6 << 8) == 0 && s != 0) {
			std::cout << ")\", R\"(";
		}

		uint64_t* addr = reinterpret_cast<uint64_t*>(src + s);
		uint64_t b = *addr;

		// 64 characters(use 35 ~ 98) : 6 bits
		for (int i = 0; i < 8; i++) {
			char c = (b & 63) + 35;
			b >>= 6;
			std::cout << c;
		}
	}
	std::cout << ")\"}" << std::endl;
}

void decode_literal(char* dst, std::string* src, size_t size) {
	memset(dst, 0, sizeof(*dst));
	size_t s = 0;
	for (; s < size; s += 6) {
		if (s % (6 << 8) == 0 && s != 0) {
			src += 1;
		}

		// next 48(64) bits
		uint64_t b = 0;
		size_t pos = (s % (6 << 8)) / 6;

		for (int i = 7; i >= 0; i--) {
			b <<= 6;
			uint64_t c = *(src->c_str() + 8 * pos + i) - 35;
			b |= c;
		}

		uint64_t* addr = reinterpret_cast<uint64_t*>(dst + s);
		*addr |= b;
	}
}