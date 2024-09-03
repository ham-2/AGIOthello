#ifndef MOVEGEN_INCLUDED
#define MOVEGEN_INCLUDED

#include "board.h"
#include "position.h"

struct MoveList {
	Square list[256];
	Square* end;

	Bitboard b;

	void generate(Position& board);

	void show();

	int length() { return int(end - list); };

	int find_index(Square m) { 
		for (auto p = list; p != end; p++) {
			if (*p == m) { return int(p - list); }
		}
		return 0;
	}

};

#endif