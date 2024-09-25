#include "board.h"

using namespace std;

Bitboard SquareBoard[64];
Bitboard LDiagBoard[15];
Bitboard RDiagBoard[15];
#ifdef _BMI2_
#ifdef _BIGC_
uint8_t Captures[65536];
#else
uint8_t Captures[8][256];
#endif
#endif

Square parse_square(char file, char rank) {
	if (file == '0') { return NULL_MOVE; }
	return Square((rank - '1') * 8 + (file - 'a'));
}

Square parse_square(string s) {
	return parse_square(s[0], s[1]);
}

bool parse_piece(char c, Piece& p) {
	size_t idx = FEN_Pieces.find(c);
	if (idx == string::npos) { return false; }
	p = Piece(idx);
	return true;
}

void print(ostream& os, Bitboard b) {
	Square sq = A8;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			os << int((b >> sq) & 1) << " ";
			++sq;
		}
		sq += (-16);
		os << "\n";
	}
	os << "\n";
}

namespace Board {

	void init() {

		// Bitboards for Single Squares
		for (int i = A1; i < SQ_END; i++) {
			SquareBoard[i] = (1ULL << i);
		}

		// Bitboards for Diagonals
		LDiagBoard[7] = 0x0102040810204080ULL;
		RDiagBoard[7] = 0x8040201008040201ULL;
		for (int i = 0; i < 7; i++) {
			LDiagBoard[6 - i] = shift<-1>(LDiagBoard[7 - i]);
			LDiagBoard[8 + i] = shift< 1>(LDiagBoard[7 + i]);
			RDiagBoard[6 - i] = shift<-1>(RDiagBoard[7 - i]);
			RDiagBoard[8 + i] = shift< 1>(RDiagBoard[7 + i]);
		}

#ifdef _BMI2_
		// Capture Table
#ifdef _BIGC_
		// Index: 16 bits
		// = 8 bits (occupied)
		// + 8 bits (us / them if occupied, empty / move if empty)
		// 00: empty (0) 01: move (1) 10: us (256) 11: them (257)
		int sq[8];
		int idx_helper[3] = { 0, 256, 257 };
		int index;

		memset(Captures, 0, sizeof(Captures));

		for (int i = 0; i < 8; i++) { // Move Square
			for (sq[7] = 0; sq[7] < 3; sq[7]++) {
				index &= Masks[7];
				index |= idx_helper[sq[7]] << 7;
			for (sq[6] = 0; sq[6] < 3; sq[6]++) {
				index &= Masks[6];
				index |= idx_helper[sq[6]] << 6;
			for (sq[5] = 0; sq[5] < 3; sq[5]++) {
				index &= Masks[5];
				index |= idx_helper[sq[5]] << 5;
			for (sq[4] = 0; sq[4] < 3; sq[4]++) {
				index &= Masks[4];
				index |= idx_helper[sq[4]] << 4;
			for (sq[3] = 0; sq[3] < 3; sq[3]++) {
				index &= Masks[3];
				index |= idx_helper[sq[3]] << 3;
			for (sq[2] = 0; sq[2] < 3; sq[2]++) {
				index &= Masks[2];
				index |= idx_helper[sq[2]] << 2;
			for (sq[1] = 0; sq[1] < 3; sq[1]++) {
				index &= Masks[1];
				index |= idx_helper[sq[1]] << 1;
			for (sq[0] = 0; sq[0] < 3; sq[0]++) {
				index &= Masks[0];
				index |= idx_helper[sq[0]] << 0;
				
				index &= Masks[i];
				index |= (1 << i);
				sq[i] = 3;

				uint8_t capture = 0;
				// Capture to the left
				int j = i - 1;
				while (j >= 0 && sq[j] == 2) { j--; }
				if (j >= 0 && sq[j] == 1) { capture |= (0xFF >> (8 - i)) ^ (0xFF >> (7 - j)); }

				// Capture to the right
				j = i + 1;
				while (j < 8 && sq[j] == 2) { j++; }
				if (j < 8 && sq[j] == 1) { capture |= (0xFF >> (7 - i)) ^ (0xFF >> (8 - j)); }

				Captures[index] = capture;
			}
			}
			}
			}
			}
			}
			}
			}
		}
#else
		for (int c = 0; c < 8; c++) {
			for (int bits = 0; bits < 256; bits++) {
				uint8_t along;

				uint8_t cand_l = 1 << c;
				along = ~bits;
				cand_l |= along & (cand_l << 1);
				along  &= along << 1;
				cand_l |= along & (cand_l << 2);
				along  &= along << 2;
				cand_l |= along & (cand_l << 4);
				cand_l ^= 1 << c;
				cand_l = (cand_l << 1) & bits ? cand_l : 0;

				uint8_t cand_r = 1 << c;
				along = ~bits;
				cand_r |= along & (cand_r >> 1);
				along  &= along >> 1;
				cand_r |= along & (cand_r >> 2);
				along  &= along >> 2;
				cand_r |= along & (cand_r >> 4);
				cand_r ^= 1 << c;
				cand_r = (cand_r >> 1) & bits ? cand_r : 0;

				Captures[c][bits] = cand_l | cand_r;
			}
		}
#endif
#endif

#ifdef _POPCNT_HELPER_
		for (int i = 0; i < 256; i++) {
			popcnt_helper[i] = (i & 1) + ((i >> 1) & 1) + ((i >> 2) & 1) + ((i >> 3) & 1) +
				((i >> 4) & 1) + ((i >> 5) & 1) + ((i >> 6) & 1) + ((i >> 7) & 1);
		}
#endif
	}
}