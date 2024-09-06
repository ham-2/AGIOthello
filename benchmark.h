#ifndef BENCH_INCLUDED
#define BENCH_INCLUDED

#include <chrono>
#include <sstream>

#include "position.h"
#include "movegen.h"
#include "table.h"
#include "threads.h"
#include "network.h"

using namespace std;

// Move Generation / Position Benchmarks
void perft(Position* board, int depth);
void solve();

// Evaluation Benchmarks
void net_speedtest();
void net_verify();

#endif