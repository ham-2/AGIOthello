#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>

#include "eval.h"

using namespace std;

atomic<long> node_count(0);
PRNG rng = PRNG(3245356235923498ULL);

int eval(Position& board) {

	node_count++;

	int score = 0;

	score += board.get_side() ?
		(board.get_count(WHITE_P) - board.get_count(BLACK_P)) << 23 :
		(board.get_count(BLACK_P) - board.get_count(WHITE_P)) << 23;

	score += ((rng.get() & 255) << 11) + ((rng.get() & 255) << 11)
		+ ((rng.get() & 255) << 11) + ((rng.get() & 255) << 11);

	return score;
}

string eval_print(int eval) {
	if (is_mate(eval)) {
		return "mate " + to_string((eval ^ EVAL_END) >> 23);
	}
	else {
		return "cd " + to_string(((eval >> 10) * 100) >> 13);
	}
}