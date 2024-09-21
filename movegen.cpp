#include "movegen.h"

template <int T>
inline constexpr Bitboard add_moves(Bitboard us, Bitboard them, Bitboard empty)
{
	Bitboard cand = us;
	if (T > 0) {
		cand |= them & (cand << T);
		them &=        (them << T);
		cand |= them & (cand << (2 * T));
		them &=        (them << (2 * T));
		cand |= them & (cand << (4 * T));
	}
	else {
		cand |= them & (cand >> -T);
		them &=        (them >> -T);
		cand |= them & (cand >> (2 * -T));
		them &=        (them >> (2 * -T));
		cand |= them & (cand >> (4 * -T));
	}
	return shift<T>(cand & ~us) & empty;
}

void MoveList::generate(Position& board) {
	end = list;
	Piece p = board.get_side() ? WHITE_P : BLACK_P;
	
	// Rays Lookup
	//Bitboard moves = EmptyBoard;
	//Bitboard upper = ~board.get_pieces(EMPTY);
	//Bitboard lower = board.get_side() ? 
	//	board.get_pieces(BLACK_P) : board.get_pieces(WHITE_P);

	//for (int file = 0; file < 8; file++) {
	//	uint16_t idx = extract_bits(upper, FileBoard[file]) << 8;
	//	idx |= extract_bits(lower, FileBoard[file]);

	//	moves |= to_file(Captures[idx], file);
	//}
	//for (int rank = 0; rank < 8; rank++) {
	//	uint16_t idx = extract_bits(upper, RankBoard[rank]) << 8;
	//	idx |= extract_bits(lower, RankBoard[rank]);

	//	moves |= to_rank(Captures[idx], rank);
	//}
	//for (int ldiag = 0; ldiag < 15; ldiag++) {
	//	uint16_t idx = extract_bits(upper, LDiagBoard[ldiag]) << 8;
	//	idx |= extract_bits(lower, LDiagBoard[ldiag]);

	//	moves |= to_ldiag(Captures[idx], ldiag);
	//}
	//for (int rdiag = 0; rdiag < 15; rdiag++) {
	//	uint16_t idx = extract_bits(upper, RDiagBoard[rdiag]) << 8;
	//	idx |= extract_bits(lower, RDiagBoard[rdiag]);

	//	moves |= to_rdiag(Captures[idx], rdiag);
	//}

	// Bitboard Method
	Bitboard moves = EmptyBoard;
	Bitboard us = board.get_pieces(p);
	Bitboard them = board.get_pieces(~p);
	Bitboard empty = board.get_pieces(EMPTY);

	moves |= add_moves<-9>(us, them & ~FileBoard[7], empty);
	moves |= add_moves<-8>(us, them, empty);
	moves |= add_moves<-7>(us, them & ~FileBoard[0], empty);
	moves |= add_moves<-1>(us, them & ~FileBoard[7], empty);
	moves |= add_moves< 1>(us, them & ~FileBoard[0], empty);
	moves |= add_moves< 7>(us, them & ~FileBoard[7], empty);
	moves |= add_moves< 8>(us, them, empty);
	moves |= add_moves< 9>(us, them & ~FileBoard[0], empty);

	b = moves;

	while (moves) {
		*end = pop_lsb(&moves);
		end++;
	}
}

void MoveList::show() {
	for (Square* m = list; m < end; m++) {
		std::cout << *m << " ";
	}
	std::cout << endl;
}