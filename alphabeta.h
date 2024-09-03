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

int alpha_beta(Position& board, atomic<bool>* stop,
	int ply, TTEntry* probe, int step,
	int alpha = EVAL_MIN, int beta = EVAL_MAX);

#endif