#include "movegen.h"

void MoveList::generate(Position& board) {
	Square* list_ptr = list;
	Piece p = board.get_side() ? WHITE_P : BLACK_P;

	Bitboard cand = EmptyBoard;
	Bitboard moves = EmptyBoard;
	Bitboard us = board.get_pieces(p);
	Bitboard them = board.get_pieces(~p);
	Bitboard empty = board.get_pieces(EMPTY);

	for (int dir : { -9, -8, -7, -1, 1, 7, 8, 9 }) {
		cand = us;
		cand = shift(cand, dir) & them;
		for (int i = 0; i < 6; i++) {
			cand = shift(cand, dir);
			moves |= cand & empty;
			cand &= them;
		}
	}

	b = moves;

	while (moves) {
		*list_ptr = pop_lsb(&moves);
		list_ptr++;
	}

	end = list_ptr;
}

void MoveList::show() {
	for (Square* m = list; m < end; m++) {
		std::cout << *m << " ";
	}
	std::cout << endl;
}