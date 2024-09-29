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
double test_net(int threads, int games, int depth_start, int depth_search, int type, Net* n);
void test_batch(string dir, int end, int threads, int games, int depth_start, int depth_search);

int find_best(Position& board, int depth,
	int alpha, int beta);

#endif