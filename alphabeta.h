#ifndef ALPHABETA_INCLUDED
#define ALPHABETA_INCLUDED

#include <mutex>
#include <atomic>
#include <iostream>

#include "eval.h"
#include "position.h"
#include "movegen.h"
#include "table.h"

using namespace std;

struct SearchParams {
	Position* board;
	atomic<bool>* stop;
	TT* table;
	int step;
};

int alpha_beta(SearchParams* sp, TTEntry* probe, 
	int ply, int alpha = EVAL_MIN, int beta = EVAL_MAX);

#endif