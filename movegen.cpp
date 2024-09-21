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
	/*Bitboard moves = EmptyBoard;
		
	int idx = 0;
	uint16_t ray = 0;
	for (int file = 0; file < 8; file++) {
		ray = board.get_files(file);
		idx = board.get_side() ? ray ^ (ray >> 8) : ray;
		
		moves |= to_file(Captures[idx], file);
	}
	for (int rank = 0; rank < 8; rank++) {
		ray = board.get_ranks(rank);
		idx = board.get_side() ? ray ^ (ray >> 8) : ray;

		moves |= to_rank(Captures[idx], rank);
	}
	for (int ldiag = 0; ldiag < 15; ldiag++) {
		ray = board.get_ldiags(ldiag);
		idx = board.get_side() ? ray ^ (ray >> 8) : ray;

		moves |= to_ldiag(Captures[idx], ldiag);
	}
	for (int rdiag = 0; rdiag < 15; rdiag++) {
		ray = board.get_rdiags(rdiag);
		idx = board.get_side() ? ray ^ (ray >> 8) : ray;

		moves |= to_rdiag(Captures[idx], rdiag);
	}*/

	// Bitboard Method
	Bitboard cand = EmptyBoard;
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

	//cand = us;
	//cand = shift<-9>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<-9>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<-8>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<-8>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<-7>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<-7>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<-1>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<-1>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<1>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<1>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<7>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<7>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<8>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<8>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}
	//cand = us;
	//cand = shift<9>(cand) & them;
	//for (int i = 0; i < 6; i++) {
	//	cand = shift<9>(cand);
	//	moves |= cand & empty;
	//	cand &= them;
	//}

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