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

	score += get_material(board);

	score += ((rng.get() & 255) << 12) + ((rng.get() & 255) << 12);

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