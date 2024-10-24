#ifndef BOOK_INCLUDED
#define BOOK_INCLUDED

#include "alphabeta.h"

constexpr uint64_t mask_upper56 = (~0ULL) ^ (~0ULL >> 56);

inline uint64_t make_entry(Key key, Square nmove) {
	return (key & mask_upper56) | nmove;
}

void build_book(int depth);

#endif
