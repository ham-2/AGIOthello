#ifndef BENCH_INCLUDED
#define BENCH_INCLUDED

#include <chrono>

#include "position.h"
#include "movegen.h"

using namespace std;

void perft(Position* board, int depth);

#endif