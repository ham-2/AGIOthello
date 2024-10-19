#ifndef MISC_INCLUDED
#define MISC_INCLUDED

#include <cstdint>

// Macros

#if defined _WIN64 && defined _MSC_VER 
#define _BITSCAN_
#include <intrin.h>
#define _POPCNT64_
#include <nmmintrin.h>
#elif defined __GNUC__
#include <x86intrin.h>
#endif

#ifdef _BMI2_
#include <immintrin.h>
inline uint64_t deposit_bits(uint8_t src, uint64_t mask) {
	return _pdep_u64(src, mask);
}
inline uint64_t extract_bits(uint64_t src, uint64_t mask) {
	return _pext_u64(src, mask);
}
#endif

// Pseudorandom Number Generator

class PRNG {
private:
	uint64_t state;

public:
	PRNG(uint64_t seed) { state = seed; }

	uint64_t get() {
		state ^= state << 13;
		state ^= state >> 7;
		state ^= state << 17;
		return state;
	}
};

extern PRNG rng;

uint64_t hash_128i(uint64_t upper, uint64_t lower);

#endif