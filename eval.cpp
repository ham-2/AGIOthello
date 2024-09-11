#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>

#include "eval.h"

using namespace std;

atomic<long> node_count(0);

int eval(Position& board) {

	node_count++;

	int score = compute(board.get_accumulator(), board.get_net(), board.get_side());

	//score += ((rng.get() & 255) << 10) - ((rng.get() & 255) << 10);

	return score;
}

string eval_print(int eval) {
	if (is_mate(eval)) {
		return "mate " + to_string((eval ^ EVAL_END) >> (EVAL_BITS - 6));
	}
	else {
		return "cd " + to_string(((eval >> (EVAL_BITS - 19)) * 100) >> 13);
	}
}