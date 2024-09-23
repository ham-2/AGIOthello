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