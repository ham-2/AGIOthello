#ifndef BENCH_INCLUDED
#define BENCH_INCLUDED

#include <chrono>
#include <sstream>

#include "position.h"
#include "movegen.h"
#include "table.h"
#include "threads.h"

using namespace std;

void perft(Position* board, int depth);

void solve();

#endif