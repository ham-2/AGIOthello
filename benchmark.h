#ifndef BENCH_INCLUDED
#define BENCH_INCLUDED

#include <chrono>
#include <sstream>
#include <cmath>

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
void test_net_rg(int games, int depth_start, int depth_search, bool g);
void test_net_n(int games, int depth_start, int depth_search, Net* n);

int find_best(Position& board, int depth,
	int alpha, int beta);

#endif