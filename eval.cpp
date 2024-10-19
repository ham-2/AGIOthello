#include <iostream>
#include <cmath>
#include <string>
#include <cstdlib>

#include "eval.h"

using namespace std;

atomic<long> node_count(0);

int eval(Position& board) {

	node_count++;

	int v[SIZE_OUT];
	board.get_eval(v);
	int score = v[0] * board.get_count_empty()
		+ v[1] * (64 - board.get_count_empty());
	score >>= 6;

	score = score > EVAL_END ? EVAL_END :
		score < -EVAL_END ? -EVAL_END : score;

	return score;
}

string eval_print(int eval) {
	if (is_mate(eval)) {
		return "mate " + to_string((eval ^ EVAL_END) >> (EVAL_BITS - 6));
	}
	else {
		return "cd " + to_string((eval * 100) >> (EVAL_BITS - 6));
	}
}