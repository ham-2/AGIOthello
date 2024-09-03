#ifndef BOARD_INCLUDED
#define BOARD_INCLUDED

#include <cstdint>
#include <cstdlib>
#include <iostream>

#include "misc.h"

enum Square : int {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	SQ_END
};

inline Square operator+(Square s, int i) { return Square(int(s) + i); }
inline Square operator-(Square s, int i) { return Square(int(s) - i); }
inline Square& operator++(Square& s) { return s = Square(int(s) + 1); }
inline Square& operator+=(Square& s, int i) { return s = Square(int(s) + i); }

Square parse_square(char file, char rank);
Square parse_square(std::string s);

inline int get_file(Square s) { return s & 7; }
inline int get_rank(Square s) { return s >> 3; }
inline int get_ldiag(Square s) { return get_file(s) + get_rank(s); }
inline int get_rdiag(Square s) { return get_file(s) - get_rank(s) + 7; }
constexpr uint8_t idx_ldiag[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	1, 1, 1, 1, 1, 1, 1, 0,
	2, 2, 2, 2, 2, 2, 1, 0,
	3, 3, 3, 3, 3, 2, 1, 0,
	4, 4, 4, 4, 3, 2, 1, 0,
	5, 5, 5, 4, 3, 2, 1, 0,
	6, 6, 5, 4, 3, 2, 1, 0,
	7, 6, 5, 4, 3, 2, 1, 0
};
constexpr uint8_t idx_rdiag[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 1, 1, 1, 1, 1, 1, 1,
	0, 1, 2, 2, 2, 2, 2, 2,
	0, 1, 2, 3, 3, 3, 3, 3,
	0, 1, 2, 3, 4, 4, 4, 4,
	0, 1, 2, 3, 4, 5, 5, 5,
	0, 1, 2, 3, 4, 5, 6, 6,
	0, 1, 2, 3, 4, 5, 6, 7
};
inline int get_ldiag_idx(Square s) { 
	return idx_ldiag[s];
}
inline int get_rdiag_idx(Square s) {
	return idx_rdiag[s];
}

typedef uint64_t Bitboard;
constexpr Bitboard EmptyBoard = 0;
constexpr Bitboard FullBoard  = ~EmptyBoard;
	
extern Bitboard SquareBoard[64];

constexpr Bitboard FileBoard[8] = {
	0x0101010101010101ULL,
	0x0101010101010101ULL << 1,
	0x0101010101010101ULL << 2,
	0x0101010101010101ULL << 3,
	0x0101010101010101ULL << 4,
	0x0101010101010101ULL << 5,
	0x0101010101010101ULL << 6,
	0x0101010101010101ULL << 7
};

constexpr Bitboard RankBoard[8] = {
	0xFFULL      , 0xFFULL << 8 , 0xFFULL << 16, 0xFFULL << 24,
	0xFFULL << 32, 0xFFULL << 40, 0xFFULL << 48, 0xFFULL << 56
};

extern Bitboard LDiagBoard[15];
extern Bitboard RDiagBoard[15];

constexpr Bitboard BoardEdges = FileBoard[0] | FileBoard[7] | RankBoard[0] | RankBoard[7];

constexpr uint16_t Masks[8] = {
	~257, ~(257 << 1), ~(257 << 2), ~(257 << 3),
	~(257 << 4), ~(257 << 5), ~(257 << 6), ~(257 << 7)
};
extern uint8_t Captures[65536];

inline Bitboard get_fileboard(Square s) { return FileBoard[get_file(s)]; }
inline Bitboard get_rankboard(Square s) { return RankBoard[get_rank(s)]; }

// Deposit functions to convert Captures value to Bitboard
inline Bitboard to_rank(uint8_t src, int index) {
	return Bitboard(src) << (8 * index);
}
inline Bitboard to_file(uint8_t src, int index) {
	return deposit_bits(src, FileBoard[index]);
}
inline Bitboard to_ldiag(uint8_t src, int index) {
	return deposit_bits(src, LDiagBoard[index]);
}
inline Bitboard to_rdiag(uint8_t src, int index) {
	return deposit_bits(src, RDiagBoard[index]);
}

#if defined _POPCNT64_
inline int popcount(Bitboard b) {
	return (int)_mm_popcnt_u64(b);
}
#elif defined __GNUC__
inline int popcount(Bitboard b) {
	return (int)__builtin_popcount(b)
}
#else
#define _POPCNT_HELPER_
uint8_t popcnt_helper[256] = {};
inline int popcount(Bitboard b) {
	return popcnt_helper[b & 255] + popcnt_helper[(b >> 8) & 255] +
		popcnt_helper[(b >> 16) & 255] + popcnt_helper[(b >> 24) & 255] +
		popcnt_helper[(b >> 32) & 255] + popcnt_helper[(b >> 40) & 255] +
		popcnt_helper[(b >> 48) & 255] + popcnt_helper[(b >> 56) & 255];
}
#endif

inline Square lsb_square(Bitboard b) {
#ifdef _BITSCAN_
	unsigned long _index = 0;
	_BitScanForward64(&_index, b);
	return Square(_index);
#else
	return Square(__builtin_ctzll(b));
#endif
}

inline Square pop_lsb(Bitboard* b) {
	Square s = lsb_square(*b);
	*b &= *b - 1;
	return s;
}

inline Bitboard shift(Bitboard b, int shift) {
	switch (shift) {
	case -9: {
		return (b & ~FileBoard[0]) >> 9;
	}
	case -8: {
		return b >> 8;
	}
	case -7: {
		return (b & ~FileBoard[7]) >> 7;
	}
	case -1: {
		return (b & ~FileBoard[0]) >> 1;
	}
	case 1: {
		return (b & ~FileBoard[7]) << 1;
	}
	case 7: {
		return (b & ~FileBoard[0]) << 7;
	}
	case 8: {
		return b << 8;
	}
	case 9: {
		return (b & ~FileBoard[7]) << 9;
	}
	default: {
		return 0;
	}
	}
}

enum Color : bool { BLACK = false, WHITE = true };

inline constexpr Color operator~(Color c) { return Color(c ^ 1); }

const std::string FEN_Pieces = " @OX";
enum Piece : char { EMPTY = 0, BLACK_P = 1, WHITE_P = 2, MISC = 3 };

inline constexpr Piece operator~(Piece p) { return Piece(p ^ 3); }
inline char print_piece(Piece p) { return FEN_Pieces[p]; }
bool parse_piece(char c, Piece& p);

inline std::ostream& operator<<(std::ostream& os, Square s) {
	return os << char('a' + get_file(s)) << 1 + get_rank(s);
}
void print(std::ostream& os, Bitboard b);

namespace Board {
	void init();
}

#endif
